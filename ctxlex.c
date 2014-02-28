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

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ctxlex.h"

struct lexstats *
lexstats_new()
{
    struct lexstats *ls = malloc(sizeof *ls);
    ls->max_wlen = 0;
    ls->nalloc = BUFSIZ;
    ls->wldist =  calloc(ls->nalloc, sizeof (*ls->wldist));
    ls->ctxldist = calloc(ls->nalloc, sizeof (*ls->ctxldist));
    ls->wdist = prob_dist_new();
    ls->ctxdist = prob_dist_new();
    return(ls);
}

void 
lexstats_free(struct lexstats *ls)
{
    int i;
    for (i = 0; i < ls->max_wlen; i++) {
        prob_dist_free(ls->wldist[i]);
        prob_dist_free(ls->ctxldist[i]);
    }
    if (ls->wdist) prob_dist_free(ls->wdist);
    if (ls->ctxdist) prob_dist_free(ls->wdist);
    if (ls->wdist) free (ls->wldist);
    if (ls->ctxdist) free (ls->ctxldist);
    free(ls);
}

enum update_type {
    UTYPE_WFREQ,
    UTYPE_CTXFREQ
};

static inline void
dist_update(struct lexstats *ls, int len, size_t val, int type)
{
    struct prob_dist *pd;
    int didx = len - 1;

    if(len >= ls->nalloc) {
        int i;
        ls->wldist = realloc(ls->wldist, 
                             (ls->nalloc + len + 1) * sizeof (*ls->wldist));
        ls->ctxldist = realloc(ls->ctxdist, 
                               (ls->nalloc + len + 1) * sizeof (*ls->ctxldist));
        for (i = ls->nalloc; i < ls->nalloc + len + 1; i++) {
            ls->wldist = NULL;
            ls->ctxldist = NULL;
        }
        ls->nalloc += len + 1;
    }
    // type == 0 -> words, otherwise context
    if (type == UTYPE_WFREQ) {
        if (ls->wldist[didx] == NULL) ls->wldist[didx] = prob_dist_new();
        pd = ls->wldist[didx];
    } else {
        if (ls->ctxldist[didx] == NULL) ls->ctxldist[didx] = prob_dist_new();
        pd = ls->ctxldist[didx];
    }

    if (val == 1) { //this is a new word or context
        prob_dist_update(pd, 1);
    } else {
        prob_dist_replace(pd, val - 1, val);
    }

}

void
lexstats_update_f(struct lexstats *ls, int len, int freq)
{
    dist_update(ls, len, freq, UTYPE_WFREQ);
}

void
lexstats_update_ctx(struct lexstats *ls, int len, int ctx)
{
    dist_update(ls, len, ctx, UTYPE_CTXFREQ);
}

struct ctxlex *
ctxlex_new()
{
    struct ctxlex *l = malloc (sizeof (*l));

    l->lex = NULL;
    l->lexhash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, free);
    l->ctxhash = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
    l->wntok = l->nctx = l->wntyp = 0;
    l->stats = lexstats_new();
    l->n = l->nalloc = 0;
    
    return l;
}


static gint32
lex_insert(struct ctxlex *lex, char *s)
{
    gint32 *i = NULL;
    assert (lex != NULL);
    i = g_hash_table_lookup(lex->lexhash, s);
    if (i == NULL) {
        struct lexdata *tmp = NULL;
        if (lex->nalloc <= lex->n) {
            lex->nalloc += BUFSIZ;
            lex->lex = realloc(lex->lex, 
                               lex->nalloc * sizeof (struct lexdata *));
        }
        tmp = malloc(sizeof (*tmp));
        tmp->freq = 0;
        tmp->nctx = 0;
        tmp->s = strdup(s);
        tmp->freq = 0;
        tmp->ctx = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, free);
        lex->lex[lex->n] = tmp;
        i = malloc(sizeof (*i));
        *i = lex->n;
        g_hash_table_insert(lex->lexhash, tmp->s, i);
        lex->n++;
    }
    return *i;
}

static inline size_t
ctx_insert(GHashTable *ctxhash, gint32 l, gint32 r)
{
    size_t *freq;
    gint64 *ctx = malloc (sizeof (*ctx));
    gint64 tmp = (gint64) l;
    *ctx =  tmp << 32;
    tmp = (gint64) r;
    *ctx |= tmp;

    assert (ctxhash != NULL);

    freq = g_hash_table_lookup(ctxhash, ctx);

    if (freq == NULL) {
        freq = malloc (sizeof (*freq));
        *freq = 1;
        g_hash_table_insert(ctxhash, ctx, freq);
    } else {
        *freq = *freq + 1;
        free(ctx);
    }
    return *freq;
}

struct lexdata *
ctxlex_add(struct ctxlex *lex, char *s, char *l, char *r)
{
    size_t tmpfreq;
    gint32 i = lex_insert(lex, s),
         i_l = lex_insert(lex, l), 
         i_r = lex_insert(lex, r);

    lex->wntok++;

    tmpfreq = ctx_insert(lex->ctxhash, i_l, i_r);
    if (tmpfreq == 1) {
        lex->nctx++;
    } 

    tmpfreq = ctx_insert(lex->lex[i]->ctx, i_l, i_r);
    if (tmpfreq == 1) {
        lex->lex[i]->nctx++;
        lexstats_update_ctx(lex->stats, strlen(s), lex->lex[i]->nctx); 
    } 
    lex->lex[i]->freq++;
    lexstats_update_f(lex->stats, strlen(s), lex->lex[i]->freq); 

    return lex->lex[i];
}

struct lexdata *
ctxlex_lookup(struct ctxlex *lex, char *s)
{
    assert (lex != NULL);
    if (lex->lex == NULL) return NULL;
    int *i = g_hash_table_lookup(lex->lexhash, s);

    return (i) ? lex->lex[*i] : NULL;
}


size_t 
ctxlex_freq(struct ctxlex *lex, char *s)
{
    struct lexdata *tmp;

    assert (lex != NULL);
    if ((tmp = ctxlex_lookup(lex, s))) {
        return tmp->freq;
    } else {
        return 0;
    }

}

double
ctxlex_freq_z(struct ctxlex *lex, char *s)
{
    double freq = (double) ctxlex_freq(lex, s);
    int len = strlen(s);

    if (freq == 0) return -INFINITY;
    return z_score(lex->stats->wldist[len - 1], freq);
}

size_t 
ctxlex_nctx(struct ctxlex *lex, char *s)
{
    struct lexdata *tmp;

    assert (lex != NULL);
    if ((tmp = ctxlex_lookup(lex, s))){
        return tmp->nctx;
    } else {
        return 0;
    }
}

double
ctxlex_nctx_z(struct ctxlex *lex, char *s)
{
    double nctx = (double) ctxlex_nctx(lex, s);
    int len = strlen(s);

    if (nctx == 0) return -INFINITY;
    return z_score(lex->stats->ctxldist[len - 1], nctx);
}


void 
ctxlex_free(struct ctxlex *lex)
{
    int i;

    g_hash_table_destroy(lex->lexhash);
    g_hash_table_destroy(lex->ctxhash);
    for (i = 0; i < lex->n; i++) {
        g_hash_table_destroy(lex->lex[i]->ctx);
        free(lex->lex[i]->s);
        free(lex->lex[i]);
    }
    free(lex->lex);
    free(lex);
}


#ifdef __CTXLEX_TEST
int
main()
{
    struct ctxlex *l = ctxlex_new();

    ctxlex_add(l, "a","b","c");
    ctxlex_add(l, "a","b","c");
    ctxlex_add(l, "a","x","c");
    ctxlex_add(l, "a","a","c");
    ctxlex_add(l, "x","b","c");
    ctxlex_add(l, "b","b","c");

    GHashTableIter iter;
    g_hash_table_iter_init (&iter, l->lexhash);
    char *key; 
    int  *i;
    while (g_hash_table_iter_next (&iter, (gpointer*)&key, (gpointer*)&i)) {
        struct lexdata *val = l->lex[*i];
        printf("%s: ", val->s); 
        printf("%zu ", val->freq );
        printf("%zu\n", val->nctx);
        if (val->nctx > 0) {
            GHashTableIter iter2;
            g_hash_table_iter_init (&iter2, val->ctx);
            gint64 *key2; 
            size_t  *freq;
            while (g_hash_table_iter_next (&iter2, (gpointer*)&key2, (gpointer*)&freq)) {
                unsigned left = (unsigned) ((*key2) >> 32),
                         right = (unsigned) ((*key2) & 0x00000000fffffff);
                printf("\t%u(%s),%u(%s) = %zu\n", left , l->lex[left]->s, right, l->lex[right]->s, *freq);
            }
        }
    }

    printf("\nctx list:\n");
    g_hash_table_iter_init (&iter, l->ctxhash);
    gint64 *key2; 
    size_t *freq;
    while (g_hash_table_iter_next (&iter, (gpointer *)&key2, (gpointer *)&freq)) {
        unsigned left = (unsigned) ((*key2) >> 32),
                 right = (unsigned) ((*key2) & 0x00000000fffffff);
        printf("\t%u(%s),%u(%s) = %zu\n", left , l->lex[left]->s, right, l->lex[right]->s, *freq);
    }

    ctxlex_free(l);
    return 0;
}
#endif
