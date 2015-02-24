#include <stdio.h>
#include <string.h>
#include "input.h"
#include "strutils.h"

void input_add_utterance(struct input *inp, 
        const char *str_phon, 
        const char *str_stress); 

struct utterance *tokenize(struct input *inp, 
        const char *str_phon, 
        const char *str_stress);

segunit_t sigma_add_phon(struct input *inp, const char *p);
segunit_t sigma_add_syl(struct input *inp, const char *p);
segunit_t sigma_add(struct input *inp, const char *p, uint8_t type);

struct input *input_read(const char *finput, 
        const char *fstress, 
        const unsigned mode)
{
    FILE            *fp = NULL, *sfp = NULL;
    struct input    *ret;

    if (!strcmp("-", finput)) {
        fp = stdin;
    } else {
        fp = fopen(finput, "r");
        if (fp == NULL)  {
            fprintf(stderr,"cannot open `%s' for reading\n", finput);
            exit(-1);
        }
    }

    if (fstress != NULL) {
        sfp = fopen(fstress, "r");
        if (fp == NULL)  {
            fprintf(stderr, "cannot open `%s' for reading\n", fstress);
            exit(-1);
        }
    }
    
    ret = malloc(sizeof *ret);
    ret->u = NULL;
    ret->sigma = NULL;
    ret->hash_phon = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    ret->hash_syl = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    ret->len = 0;
    ret->alloc = 0;
    ret->sigma_nph = 0;
    ret->sigma_nsyl = 0;
    ret->sigma_alloc = 0;
    ret->sigma_idx = NULL;
    ret->mode = mode;
    ret->file_name = finput;
    ret->sfile_name = fstress;

    sigma_add_phon(ret, SYM_BOS);
    sigma_add_phon(ret, SYM_EOS);
    sigma_add_syl(ret, SYM_BOS);
    sigma_add_syl(ret, SYM_EOS);

    do {
        char *buf_phon = NULL;
        char *buf_stress = NULL;
        if (feof(fp)) 
            break;

        fscanf(fp, "%m[^\n]\n", &buf_phon);
        if (*buf_phon == COMMENT_CHAR) {
            free(buf_phon);
            continue;
        }

        if (sfp != NULL) {
            fscanf(sfp, "%m[^\n]\n", &buf_stress);
            while (*buf_stress == COMMENT_CHAR) {
                free(buf_stress);
                fscanf(sfp, "%m[^\n]\n", &buf_stress);
            }
        }

        str_strip(buf_phon, " \t\n");
        if (buf_stress != NULL) {
            str_strip(buf_stress, " \t\n");
        }
        input_add_utterance(ret, buf_phon, buf_stress);

        if (buf_phon) free(buf_phon);
        if (buf_stress) free(buf_stress);
    } while (!feof(fp));

    return ret;
}

void input_add_utterance(struct input *inp, 
        const char *str_phon, 
        const char *str_stress) 
{

    if (inp->alloc <= inp->len) {
        struct utterance **tmp = NULL;

        inp->alloc += BUFSIZ;
        tmp = realloc(inp->u, inp->alloc * (sizeof (struct uttterance *)));
        if (tmp) {
           inp->u = tmp;
        } else {
            fprintf(stderr, "unable to allocate memory\n");
            exit(-1);
        }
    }

    inp->u[inp->len] = tokenize(inp, str_phon, str_stress);

    inp->len += 1;

}

struct utterance *tokenize(struct input *inp, 
        const char *str_phon, 
        const char *str_stress) 
{
    const char *p = str_phon;
    size_t maxlen = strlen(str_phon) + 3;
    struct utterance *u = malloc(sizeof *u);

    u->phon = malloc(maxlen * sizeof *(u->phon));
    u->feat = malloc(maxlen * sizeof *(u->feat));
    u->nphon = 0;
    u->nsyl = 0;
    u->gs_seg = malloc(maxlen * sizeof *(u->gs_seg));
    u->gs_seg[0] = 0;
    u->syl = NULL;
    u->syl_seg = NULL;
    if (inp->mode & (INPMODE_SYL | INPMODE_PSYL)) {
        u->syl = malloc(maxlen * sizeof *(u->phon));
        u->syl_seg = malloc(maxlen * sizeof *(u->syl_seg));
        u->syl_seg[0] = 0;
    } 


    uint8_t stress = 0;
    bool syl_boundary = false;
    char syl_tmp[maxlen];
    syl_tmp[0] = '\0';
    while(*p) {
        char sym[5];
        unsigned char first_byte = p[0];
        if((first_byte >> 7) == 0x00) { // 1 byte
            sym[0] = p[0]; sym[1] = '\0';
            p += 1;
        } else if((first_byte >> 5) == 0x06) { // 2 bytes
            strncpy(sym, p, 2); sym[2] = '\0';
            p += 2;
        } else if((first_byte >> 4) == 0x0e) { // 3 bytes
            strncpy(sym, p, 3); sym[3] = '\0';
            p += 3;
        }  else if((first_byte >> 3) == 0x1e) { // 4 bytes
            strncpy(sym, p, 4); sym[4] = '\0';
            p += 4;
        } else {// something is wrong, 
            strcpy(sym,"�"); // U+FFFD Replacement character
            p += 1; // try to recover from the next character
        }

        if (*sym == ' ' || *sym == '\t') { // boundary 
            u->gs_seg[0] += 1;
            u->gs_seg[u->gs_seg[0]] = u->nphon;
            while (*(p + 1) == ' ' || *(p + 1) == '\t') p++;
            if (inp->mode * INPMODE_SYL) {
                u->syl_seg[0] += 1;
                u->syl_seg[u->syl_seg[0]] = u->nphon;
                syl_boundary = true;
            }
        } else if ((inp->mode * INPMODE_SYL) && *sym == '.') {
            u->syl_seg[0] += 1;
            u->syl_seg[u->syl_seg[0]] = u->nphon;
            syl_boundary = true;
        } else if (!strcmp("ˈ", sym)) { // primary stress
            stress = PHONFEAT_STRESS1;
        } else if (!strcmp("ˌ", sym)) { // secondary stress
            stress = PHONFEAT_STRESS2;
        // TODO: diacratic processing goes in here.
        } else {
            // TODO: deal with pseudo-syllabification here.

            u->phon[u->nphon] = sigma_add_phon(inp, sym);
            u->feat[u->nphon] |= stress;
            if (inp->mode & (INPMODE_SYL | INPMODE_PSYL)) {
                if(syl_boundary) {
                    u->syl[u->nsyl] = sigma_add_syl(inp, syl_tmp);
                    syl_tmp[0]= '\0';
                    u->nsyl += 1;
                    stress = 0;
                    syl_boundary = false;
                }
                strcat(syl_tmp, sym);
                if (*p == '\0') {
                    u->syl[u->nsyl] = sigma_add_syl(inp, syl_tmp);
                    u->nsyl += 1;
                }
            } else {
                stress = 0;
            }
            u->nphon += 1;
        }
    }

    u->phon[u->nphon] = 0;
    u->feat[u->nphon] = 0;

    u->phon = realloc(u->phon, (u->nphon + 1) * sizeof *(u->phon));
    u->feat = realloc(u->feat, (u->nphon + 1) * sizeof *(u->feat));
    if (u->syl != NULL) {
        u->syl[u->nsyl] = 0;
        u->syl = realloc(u->syl, (u->nsyl + 1) * sizeof *(u->syl));
    }
    return u;
}

#define SIGMA_PHON 1
#define SIGMA_SYL  2

segunit_t sigma_add_phon(struct input *inp, const char *p)
{
    size_t *idx = g_hash_table_lookup(inp->hash_phon, p);

    if (idx != NULL) {
        return *idx;
    } else  {
        inp->sigma_nph += 1;
        return sigma_add(inp, p, SIGMA_PHON);
    }
}

segunit_t sigma_add_syl(struct input *inp, const char *p)
{
    size_t *idx = g_hash_table_lookup(inp->hash_syl, p);

    if (idx != NULL)  {
        return *idx;
    } else {
        inp->sigma_nsyl += 1;
        return sigma_add(inp, p, SIGMA_SYL);
    }
}

segunit_t sigma_add(struct input *inp, const char *p, uint8_t type)
{

    // NOTE: sigma[0] is special, not used.
    if (inp->sigma_alloc <= (inp->sigma_len + 1)) {
        struct basic_unit *tmp = NULL;
        segunit_t  *tmpidx = NULL;

        inp->sigma_alloc += BUFSIZ;
        tmp = realloc(inp->sigma, 
                inp->sigma_alloc * (sizeof *(inp->sigma)));
        if (tmp) {
           inp->sigma = tmp;
        } else {
            fprintf(stderr, "unable to allocate memory\n");
            exit(-1);
        }

        tmpidx = realloc(inp->sigma_idx, 
                inp->sigma_alloc * (sizeof *(inp->sigma_idx)));
        if (tmpidx) {
            size_t i;
            inp->sigma_idx = tmpidx;
            for (i = 0; i < BUFSIZ; i++) {
                inp->sigma_idx[inp->sigma_alloc - i - 1] = 
                    inp->sigma_alloc - i - 1;
            }
        } else {
            fprintf(stderr, "unable to allocate memory\n");
            exit(-1);
        }
    }

    inp->sigma_len += 1;
    
    inp->sigma[inp->sigma_len].str = strdup(p);
    inp->sigma[inp->sigma_len].feat = 0;

    if (type == SIGMA_PHON) {
        g_hash_table_insert(inp->hash_phon, inp->sigma[inp->sigma_len].str, 
                &(inp->sigma_idx[inp->sigma_len]));
    } else if(type == SIGMA_SYL) {
        g_hash_table_insert(inp->hash_syl, inp->sigma[inp->sigma_len].str, 
                &(inp->sigma_idx[inp->sigma_len]));
        inp->sigma[inp->sigma_len].feat |= PHONFEAT_ISSYL;
    }

    return inp->sigma_len;
}


/* Shuffle the given input list */
void input_shuffle(struct input *in, unsigned seed)
{
    int i;

    if (seed == 0) {
        srand(time(NULL));
    } else {
        srand(seed);
    }

    for (i = 0; i < in->len - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (in->len - i) + 1);
        struct utterance *t = in->u[j];
        in->u[j] = in->u[i];
        in->u[i] = t;
    }
}

static void free_sigma(struct basic_unit *u, size_t len)
{
    size_t i;
    for (i = 1; i <= len; i++) {
        free(u[i].str);
    }
    free(u);
}

static void free_utterance(struct utterance *u)
{
    if (u->phon) free(u->phon);
    if (u->syl) free(u->syl);
    if (u->gs_seg) free(u->gs_seg);
    if (u->syl_seg) free(u->syl_seg);
    if (u->feat) free(u->feat);
    free(u);
}

void input_free(struct input *in)
{
    size_t i;
    for (i = 0; i < in->len; i++) {
        free_utterance(in->u[i]);
    }
    free_sigma(in->sigma, in->len);
    if (in->hash_phon) g_hash_table_destroy(in->hash_phon);
    if (in->hash_syl)g_hash_table_destroy(in->hash_syl);
    free(in->sigma_idx);
}


struct output *output_new(size_t len)
{
    struct output *ret = malloc(sizeof *ret);

    if (len != 0) {
        ret->len = len;
        ret->alloc = len;
        ret->seglist = malloc (ret->len * sizeof ret->seglist);
    } else {
        ret->len = 0;
        ret->alloc = 0;
    }
    return ret;
}

void output_free(struct output *out, bool free_seglist)
{
    if (out->len == 0) {
        return;
    } else if (free_seglist) {
        int i;
        for (i = 0; i < out->len; i++) {
            seglist_free(out->seglist[i]);
        }
    }
    free(out->seglist);
    free(out);
}

void output_add(struct output *out, struct seglist *segs)
{
    out->len += 1;
    if(out->alloc <= out->len) {
        struct seglist  **tmp;
        out->alloc += BUFSIZ;
        tmp = realloc(out->seglist, out->alloc * (sizeof *tmp));
        if(tmp)
            out->seglist = tmp;
        else
            fprintf(stderr, "unable to allocate memory\n");
    }
    out->seglist[out->len] = segs;
}


void output_write(char *outf, struct output *out, struct input *inp)
{
    FILE *fp = NULL;
    int i = 0, j = 0, k = 0;
    if(!strcmp("-", outf)) {
        fp = stdout;
    } else {
        fp = fopen(outf, "w");
        if(fp == NULL)  {
            fprintf(stderr, "cannot open `%s' for reading\n", outf);
        }
    }

    for (i = 0; i < out->len; i++) {
        for (j = 0; j < out->seglist[i]->nsegs; j++) {
            segunit_t *seg = out->seglist[i]->segs[j];
            segunit_t phon_i = 0;
            for (k = 1; k <= seg[0]; k++) {
                while (phon_i < seg[k]) {
                    segunit_t sym_i = inp->u[i]->phon[phon_i];
                    fprintf(fp, "%s", inp->sigma[sym_i].str);
                    // TODO: (optionally) mark syllable boundaries 
                    phon_i += 1;
                }
                fprintf(fp, "%s", SEG_DELIM);
            }
            if (j < out->seglist[i]->nsegs) {
                fprintf(fp, "%s", SEGS_DELIM);
            }
        }
    }

    if(fp != stdout) fclose(fp);
}


#ifdef INPUT_TEST
int main(int argc, char **argv)
{
    struct input *in = input_read(argv[1], NULL, INPMODE_SYL);
    size_t i;

    for (i = 0; i < in->len; i++) {
        segunit_t gs_i = 1;
        segunit_t ss_i = 1;
        size_t j;

        printf("u%02zu[%hu] ", i, in->u[i]->gs_seg[0]);
        for (j = 0; j < in->u[i]->nphon; j++) {
            printf("%s(%hu) ", in->sigma[in->u[i]->phon[j]].str, 
                    in->u[i]->phon[j]);
            bool boundary = false;
            if (gs_i <= in->u[i]->gs_seg[0] && 
                    in->u[i]->gs_seg[gs_i] - 1 == j) {
                printf("|%hu ", in->u[i]->gs_seg[gs_i]);
                gs_i += 1;
                boundary = true;
            }
            if ((in->mode & (INPMODE_SYL|INPMODE_PSYL)) && 
                    ss_i <= in->u[i]->syl_seg[0] && 
                    in->u[i]->syl_seg[ss_i] - 1 == j) {
                if (!boundary) printf(". ");
                ss_i += 1;
            }
        }
        printf("\n");
    }

    return 0;
}
#endif