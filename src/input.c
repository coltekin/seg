#include <stdio.h>
#include <string.h>
#include "input.h"
#include "strutils.h"
#include "xalloc.h"


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
    ret->sigma_len = 0;
    ret->sigma_idx = NULL;
    ret->mode = mode;
    ret->file_name = finput;
    ret->sfile_name = fstress;


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

    fclose(fp);
    if (sfp != NULL) {
        fclose(sfp);
    }
    return ret;
}

void input_add_utterance(struct input *inp, 
        const char *str_phon, 
        const char *str_stress) 
{

    if (inp->alloc <= inp->len) {
        inp->alloc += BUFSIZ;
        inp->u = xrealloc(inp->u, inp->alloc * (sizeof (struct uttterance *)));
    }

    inp->u[inp->len] = tokenize(inp, str_phon, str_stress);

    inp->len += 1;

}

struct utterance *utterance_new(uint8_t mode, size_t maxlen)
{
    struct utterance *u = malloc(sizeof *u);

    u->phon = malloc(sizeof *u->phon);
    u->phon->len = 0;
    u->phon->seq = malloc(maxlen * sizeof *u->phon->seq);
    u->phon->feat = malloc(maxlen * sizeof *u->phon->feat);
    u->gs_seg = malloc(sizeof *u->gs_seg);
    u->gs_seg->bound = malloc(maxlen * sizeof *u->gs_seg->bound);
    u->gs_seg->len = 0;
    u->syl = NULL;
    u->syl_seg = NULL;
    if (mode & (INPMODE_SYL | INPMODE_PSYL)) {
        u->syl = malloc(sizeof *(u->syl));
        u->syl->len = 0;
        u->syl->seq = malloc(maxlen * sizeof *u->syl->seq);
        u->syl->feat = malloc(maxlen * sizeof *u->syl->feat);
        u->syl_seg = malloc(sizeof *u->syl_seg);
        u->syl_seg->bound = malloc(maxlen * sizeof *u->syl_seg->bound);
        u->syl_seg->len = 0;
    } 
    return u;
}

uint32_t utf8_to_utf_code_point(char *s)
{
    unsigned char c1 = s[0];
    if((c1 >> 7) == 0x00) {
        return  s[0];
    } else {
        unsigned char c2 = s[1];
        if ((c1 >> 5) == 0x06) {
            return (c1-192)*64+c2-128;
        } else {
            unsigned char c3 = s[2];
            if ((c1 >> 4) == 0x03) {
                return  (c1 - 224) * 4096 + (c2 - 128) * 64 + c3 - 128;
            } else {
                unsigned char c4 = s[3];
                return (c1 - 240) * 262144 + (c2 - 128) * 4096 
                        + (c3 - 128) * 64 + c4 - 128;
            }
        }
    }
}

size_t next_utf8_symbol(const char *s, char *sym)
{
    unsigned char c1 = s[0];
    if((c1 >> 7) == 0x00) { // 1 byte
        sym[0] = s[0]; sym[1] = '\0';
        return 1;
    } else if((c1 >> 5) == 0x06) { // 2 bytes
        strncpy(sym, s, 2); sym[2] = '\0';
        return 2;
    } else if((c1 >> 4) == 0x0e) { // 3 bytes
        strncpy(sym, s, 3); sym[3] = '\0';
        return 3;
    }  else if((c1 >> 3) == 0x1e) { // 4 bytes
        strncpy(sym, s, 4); sym[4] = '\0';
        return 4;
    } else {// something is wrong, 
        strcpy(sym,"�"); // U+FFFD Replacement character
        return 1; // try to recover from the next character
    }
}

struct utterance *tokenize(struct input *inp, 
        const char *str_phon, 
        const char *str_stress) 
{
    const char *p = str_phon;
    size_t maxlen = strlen(str_phon) + 1;
    struct utterance  *u = utterance_new(inp->mode, maxlen);

    uint8_t stress = 0;
    bool syl_boundary = false;
    char syl_tmp[maxlen];
    syl_tmp[0] = '\0';
    while(*p) {
        bool special_sym = true;
        char sym[16]; // should be enough for three utf8 characters
        p += next_utf8_symbol(p, sym);

        if (*sym == ' ' || *sym == '\t') { // boundary 
            u->gs_seg->bound[u->gs_seg->len] = u->phon->len;
            u->gs_seg->len += 1;
            while (*p == ' ' || *p == '\t') p++;
            if (inp->mode & INPMODE_SYL) {
                u->syl_seg->bound[u->syl_seg->len] = u->phon->len;
                u->syl_seg->len += 1;
                syl_boundary = true;
            }
        } else if (*sym == '.') {
            if (inp->mode & INPMODE_SYL) {
                u->syl_seg->bound[u->syl_seg->len] = u->phon->len;
                u->syl_seg->len += 1;
                syl_boundary = true;
            }
        } else if (!strcmp("ˈ", sym) || *sym == '\'') { // primary stress
            stress = SEGFEAT_STRESS1;
        } else if (!strcmp("ˌ", sym) || *sym == ',') { // secondary stress
            stress = SEGFEAT_STRESS2;
        // TODO: diacratic processing goes in here.
        } else {
            // TODO: deal with pseudo-syllabification here.

            // TODO: change the following to use unicode character class range
            //       of IPA diacritics
            /* we peek forward to see if there is a modifier 
             * for the current IPA symbol 
             * */
            if (inp->mode & INPMODE_IPA) {
                if (*p == ':') {
                    p += 1;
                } else if (!strncmp("ː", p, strlen("ː"))) {
                    strcat(sym, "ː");
                    p += strlen("ː");
                } else if (!strncmp(p, "ˑ", strlen("ˑ"))) {
                    strcat(sym, "ˑ");
                    p += strlen("ˑ");
                }
            }

            u->phon->seq[u->phon->len] = sigma_add_phon(inp, sym);
            u->phon->feat[u->phon->len] |= stress;
            u->phon->len += 1;
            special_sym = false;
            syl_boundary = false;
        }

        if (*p == '\0') {
            syl_boundary = true;
        }

        if (inp->mode & (INPMODE_SYL | INPMODE_PSYL)) {
            if (!special_sym) {
                strcat(syl_tmp, sym);
            }

            if(syl_boundary && syl_tmp[0]) {
                u->syl->seq[u->syl->len] = sigma_add_syl(inp, syl_tmp);
                u->syl->feat[u->syl->len] = SEGFEAT_ISSYL | stress;
                syl_tmp[0]= '\0';
                u->syl->len += 1;
                stress = 0;
            }
        } else {
            stress = 0;
        }

    }

    u->phon->seq = xrealloc(u->phon->seq, 
            (u->phon->len + 1) * sizeof *(u->phon->seq));
    u->phon->feat = xrealloc(u->phon->feat, 
            (u->phon->len + 1) * sizeof *(u->phon->feat));
    if (u->syl != NULL) {
        u->syl->seq = xrealloc(u->syl->seq, 
                (u->syl->len + 1) * sizeof *(u->syl->seq));
        u->syl->feat = xrealloc(u->syl->feat, 
                (u->syl->len + 1) * sizeof *(u->syl->feat));
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

        inp->sigma_alloc += BUFSIZ;
        inp->sigma = xrealloc(inp->sigma, 
                inp->sigma_alloc * (sizeof *(inp->sigma)));

        inp->sigma_idx = xrealloc(inp->sigma_idx, 
                inp->sigma_alloc * (sizeof *(inp->sigma_idx)));
        size_t i;
        for (i = 0; i < BUFSIZ; i++) {
            inp->sigma_idx[inp->sigma_alloc - i - 1] = 
                inp->sigma_alloc - i - 1;
        }
    }

    /* beginning/end of sequence symbols are added only to the lexicon,
     * not to the reverse lookup tables 
     */
    if (inp->sigma_len == 0) {
        inp->sigma[1].str = strdup(SYM_BOS);
        inp->sigma[1].feat = 0;
        inp->sigma[2].str = strdup(SYM_EOS);
        inp->sigma[2].feat = 0;
        inp->sigma_len += 2;
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
        inp->sigma[inp->sigma_len].feat |= SEGFEAT_ISSYL;
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
    if (u->phon) {
        free(u->phon->seq);
        free(u->phon->feat);
        free(u->phon);
    }
    if (u->syl) {
        free(u->syl->seq);
        free(u->syl->feat);
        free(u->syl);
    }
    if (u->gs_seg) {
        free(u->gs_seg->bound);
        free(u->gs_seg);
    }
    if (u->syl_seg) {
        free(u->syl_seg->bound);
        free(u->syl_seg);
    }
    free(u);
}

void input_free(struct input *in)
{
    size_t i;
    for (i = 0; i < in->len; i++) {
        free_utterance(in->u[i]);
    }
    free(in->u);
    free_sigma(in->sigma, in->sigma_len);
    if (in->hash_phon) g_hash_table_destroy(in->hash_phon);
    if (in->hash_syl)g_hash_table_destroy(in->hash_syl);
    free(in->sigma_idx);
    free(in);
}


static char *append_str(char *buf, char *str, size_t *idx, size_t *buf_alloc)
{
    char *p = str;
    while(*p) {
        if (*buf_alloc <= (*idx + 1)) {
            *buf_alloc += BUFSIZ;
            buf = xrealloc(buf, *buf_alloc);
        }
        buf[*idx] = *p;
        *idx += 1;
        p += 1;
    }
    return buf;
}

char *segment_to_str(struct input *in, size_t idx, struct segmentation *seg)
{
    char *buf = NULL;
    size_t buf_alloc = 0;
    size_t buf_i = 0;
    size_t i;

    segunit_t ph_i = 0;
    for (i = 0; seg != NULL && i < seg->len; i++) {
        while (ph_i < seg->bound[i]) {
            segunit_t sym_i = in->u[idx]->phon->seq[ph_i];
            buf = append_str(buf, in->sigma[sym_i].str, &buf_i, &buf_alloc);
            ph_i += 1;
        }
        buf = append_str(buf, " ", &buf_i, &buf_alloc);
    }
    for (i = ph_i; i < in->u[idx]->phon->len; i++) {
        segunit_t sym_i = in->u[idx]->phon->seq[i];
        buf = append_str(buf, in->sigma[sym_i].str, &buf_i, &buf_alloc);
    }
    buf[buf_i] = '\0';
    return xrealloc(buf, buf_i + 1);
}

void write_segs(char *outf, struct segmentation **segs, struct input *inp)
{
    FILE *fp = NULL;
    size_t i = 0;
    if(!strcmp("-", outf)) {
        fp = stdout;
    } else {
        fp = fopen(outf, "w");
        if(fp == NULL)  {
            fprintf(stderr, "cannot open `%s' for reading\n", outf);
        }
    }

    for (i = 0; i < inp->len; i++) {
        char *s = segment_to_str(inp, i, segs[i]);
        fprintf(fp, "%s\n", s);
        free(s);
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
