/*  
    Copyright 2010-2014 Çağrı Çöltekin <c.coltekin@rug.nl>

    This file is part of seg, an application for word segmentation.

    seg is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program as `gpl.txt'. If not, see 
    <http://www.gnu.org/licenses/>.
*/

/*
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "phonstats.h"
#include "io.h"

/* phonstats_init() - initialize the phoneme statistics data
 *
 * max_ng is the maximum ngram statistics that we are 
 * want to keep. 1: only ungrams, 2: unigrams & bigrams ...
 * mote that the index numbers for n_{typ|tok} are off by one. 
 * 0: unigram.., 1: bigram...
 *
 * if phon_list is not NULL, the ngram statistics are initialized 
 * using the given string. it may be  used as providing a prior 
 * over the phoneme distribution * (e.g., uniform distribution 
 * if the set of phonemes are given)
 *
 */
struct phonstats * 
phonstats_new(size_t max_ng, char *phon_list)
{
    assert (max_ng >= 1);
    struct phonstats *ps = malloc(sizeof *ps); 
    ps->max_ng = max_ng;
    ps->n_updt = 0;
    ps->n_tok = malloc(max_ng * sizeof (*ps->n_tok));
    ps->n_typ = malloc(max_ng * sizeof (*ps->n_typ));
    ps->nalloc = malloc((max_ng) * sizeof (*ps->nalloc));
    ps->ngstr = malloc((max_ng) * sizeof (*ps->ngstr));
    ps->st = NULL;
    memset(ps->n_tok, 0, max_ng * sizeof(*ps->n_tok));
    memset(ps->n_typ, 0, max_ng * sizeof(*ps->n_typ));
    memset(ps->nalloc, 0, max_ng * sizeof(*ps->nalloc));
    memset(ps->ngstr, 0, max_ng * sizeof(*ps->ngstr));
    ps->hash = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

    if (phon_list != NULL) {
        phonstats_update(ps, phon_list);
    }
    return ps;
}

struct phonstats * 
phonstats_new_st(size_t max_ng, char *phon_list)
{
    int i;
    struct phonstats *ps = phonstats_new(max_ng, NULL);
    ps->st = malloc(max_ng * sizeof (*ps->st));
    for(i=0; i < max_ng; i++) {
        ps->st[i] = prob_dist_new();
    }
    return ps;
}

void
phonstats_free(struct phonstats *ps)
{
    int i;
    g_hash_table_destroy(ps->hash);
    for (i = 0; i < ps->max_ng; i++) {
        free(ps->ngstr[i]);
        if (ps->st) prob_dist_free(ps->st[i]);
    }
    if (ps->st) free(ps->st);
    free(ps->ngstr);
    free(ps->n_tok);
    free(ps->n_typ);
    free(ps->nalloc);
    free(ps);
}


size_t
phonstats_freq_ng(struct phonstats *ps, char *ng)
{
    size_t *freq = g_hash_table_lookup(ps->hash, ng);
    return (freq != NULL) ? *freq : 0;
}


double
phonstats_rfreq_ng(struct phonstats *ps, char *ng)
{
    int nglen = strlen(ng) - 1;
    int freq;
//printf("DDD: (%s) nglen: %d max: %d\n", ng, nglen, ps->max_ng);
    assert (nglen <= ps->max_ng);

    
    freq = phonstats_freq_ng(ps, ng);

    return (double) (freq + 1) /
           (double) (ps->n_tok[nglen] + 1);
}

size_t
phonstats_freq_p(struct phonstats *ps, char ch)
{
    char tmp[2];
    tmp[0] = ch;
    tmp[1] = '\0';
/*
    return (ch == BOW_CH || ch == EOW_CH) ? ps->n_updt : 
                                            phonstats_freq_ng(ps, tmp);
*/
    return phonstats_freq_ng(ps, tmp);
}

/* phonstats_rfreq_p() - return relative frequency of a phoneme
 * 
 * Note: (1) we use `add-one smooting'. 
 *       (2) this function NOT does consider boundaries 
 *           as phonemes.
 */
double 
phonstats_rfreq_p(struct phonstats *ps, unsigned char ch)
{

    size_t freq = phonstats_freq_p(ps, ch);

    return (double) (freq + 1) /
           (double) (ps->n_tok[NG_UNIGRAM] - 2 * ps->n_updt + 1);

//  
// commit 5089f45d0f6f6150b140c6c7aac4a56d287a5ce7 and prior version
// used to use the following formula. It makes a minor differnce.
//    return (double) (ps->ug_freq[ch] + 1) /
//           (double) (ps->n_tok[NG_UNIGRAM] + ps->n_typ[NG_UNIGRAM] + 1);
}

/* phonstats_rfreq_p2() - same as phonstats_rfreq_p, but 
 * also count boundaries as if they are phonemes.
 * 
 */
double 
phonstats_rfreq_p2(struct phonstats *ps, unsigned char ch)
{
    size_t freq = phonstats_freq_p(ps, ch);

    return (double) (freq + 1) /
           (double) (ps->n_tok[NG_UNIGRAM] + 1);
}

void
inc_ng_freq(struct phonstats *ps, int ng, char *ngstr)
{
    char *key = strdup(ngstr);
    size_t *val;

    assert(ps->hash != NULL);

    ++(ps->n_tok[ng]);
    val = g_hash_table_lookup(ps->hash, key);
    if (val != NULL) {
        ++(*val);
        free(key);
        if(ps->st) {
            prob_dist_remove(ps->st[ng], *val - 1);
            prob_dist_update(ps->st[ng], *val);
        }
    } else {
        val = malloc(sizeof *val);
        *val = 1;

        if(ps->n_typ[ng] * sizeof (key) >= ps->nalloc[ng]) {
            char **tmp;
            ps->nalloc[ng] += BUFSIZ;
            tmp = realloc(ps->ngstr[ng], ps->nalloc[ng]);
            assert(tmp != NULL);
            ps->ngstr[ng] = tmp;
        }

        ps->ngstr[ng][ps->n_typ[ng]] = key;
        ++(ps->n_typ[ng]);
        g_hash_table_insert(ps->hash, key, val);
        if (ps->st) {
            prob_dist_update(ps->st[ng], *val);
        }
    }
}


void 
phonstats_update(struct phonstats *ps, char *s)
{
    int len = strlen(s);
    char stmp[len + 3],
         ngtmp[len + 3];
    int start, ng;

    stmp[0] = BOW_CH;
    strcpy(stmp + 1, s);
    stmp[len + 1] = EOW_CH;
    stmp[len + 2] = '\0';
    len += 2;

    ++ps->n_updt;

    for (start = 0; start < len; start++) {
/*
        if (start < len - 2) { // we do not add < and > to phonemes list
            strncpy(ngtmp, s + start, 1);
            ngtmp[1] = '\0';
            inc_ng_freq(ps, NG_UNIGRAM, ngtmp);
        }
*/
        for (ng = 0; ng < ps->max_ng; ng++) {
            if (ng < len - start) {
                strncpy(ngtmp, stmp + start, ng + 1);
                ngtmp[ng + 1] = '\0';
                inc_ng_freq(ps, ng, ngtmp);
            }
        }
    }
}

double
phonstats_P(struct phonstats *ps, char *ng, int options)
{
    int nglen = strlen(ng) - 1;
    size_t freq;
    double p;


    assert (nglen <= ps->max_ng);

    freq = phonstats_freq_ng(ps, ng);
    switch (options) {
        case SMOOTH_ADD1: {
            p =  (double) (freq + 1) / (double) (ps->n_tok[nglen] + 1);
        } break;
        case SMOOTH_WB: { // 
            p = (double) freq / (double) (ps->n_tok[nglen] + ps->n_typ[nglen]);
        } break;
        default: {
            p = (double) freq / (double) ps->n_tok[nglen];
        }
    }

    return p;
}

void 
phonstats_update_from_file(struct phonstats *ps, char *fname)
{
    int i;
    struct input *in = read_input(fname);
    for (i = 0; i < in->size; i++) {
        phonstats_update(ps, in->u[i].s);
    }
    input_free(in);
}

void
phonstats_dump(struct phonstats *ps)
{
    size_t nglen = 0, ngidx = 0;
    printf("; ------------ phonstats dump\n");
    printf("; max_ng: %zu\n", ps->max_ng);
    printf("; n_updt: %zu\n", ps->n_updt);
    printf("; n_tok: ");
    for (nglen = 0; nglen < ps->max_ng; nglen++) 
        printf(" %zu", ps->n_tok[nglen]);
    printf("\n; n_typ: ");
    for (nglen = 0; nglen < ps->max_ng; nglen++) 
        printf(" %zu", ps->n_typ[nglen]);
    printf("\n");
    for (nglen = 0; nglen < ps->max_ng; nglen++) {
        printf("; ngrams size %zu\n", nglen);
        for (ngidx = 0; ngidx < ps->n_typ[nglen]; ngidx++) {
            char *ng = ps->ngstr[nglen][ngidx];
            printf("%s: %zu\n", ng, phonstats_freq_ng(ps, ng));
        }
    }
    printf("; ------------ end of phonstats dump\n");
}


double 
phonstats_nglen_mean(struct phonstats *ps, int nglen)
{
    assert (nglen <= ps->max_ng);
    return (double) ps->n_tok[nglen] / (double) ps->n_typ[nglen];
}

double phonstats_nglen_sd(struct phonstats *ps, int nglen)
{
    double var = 0.0;
    int i;
    double mean = phonstats_nglen_mean(ps, nglen);

    for (i = 0; i < ps->n_typ[nglen]; i++) {
        size_t freq = phonstats_freq_ng(ps, ps->ngstr[nglen][i]);
        var += (mean - freq) * (mean - freq);
    }

    return sqrt(var / (double) ps->n_typ[nglen]);
}

double phonstats_freq_z(struct phonstats *ps, char *ng)
{
    double freq = (double) phonstats_freq_ng(ps, ng);
    size_t nglen = strlen(ng) - 1;
    double mean, sd;

    if (ps->st) {
        mean = mean(ps->st[nglen]);
        sd = sd(ps->st[nglen]);
    } else {
        mean = phonstats_nglen_mean(ps, nglen);
        sd = phonstats_nglen_sd(ps, nglen);
    }

    return (freq - mean) / sd;
}

/* phonstats_copy() - copy a phonstats structure to another
 * all ngrams in src is added to dst.
 * if dst is a new phonstats, the final contets are the same, 
 *
 * TODO: this copy is slow
 */
void
phonstats_copy(struct phonstats *dst, struct phonstats *src)
{
    assert (dst != NULL && src != NULL);
    size_t nglen;
    for (nglen = 0; nglen < src->max_ng; nglen++) {
        int ngidx;
        for (ngidx = 0; ngidx < src->n_typ[nglen]; ngidx++) {
            char *ng = src->ngstr[nglen][ngidx];
            int freq = phonstats_freq_ng(src, ng);
            int i;
            for (i = 0; i < freq; i++)
                phonstats_update(dst, ng);
        }
    }
}
