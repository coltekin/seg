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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "score.h"
#include "trie.h"


static GHashTable  *lex_in;
static GHashTable  *lex_out;

/* get_tp_fn_fa()
 *
 * walk through two (int) segmentation lists, and return 
 * their overlap/missmatch in terms of true positves (hits), 
 * misses (false negatives), false positives (false alarms)
 *
 * NOTE: the tp(hit)/fn(miss)/fp(fa) arugments are incremented.
 */

void get_tp_fn_fp(struct seg_counts *c, 
             struct segmentation *gs, struct segmentation *res)
{
    int ngs = (gs != NULL) ? gs->len : 0, 
        nres = (res != NULL) ? res->len : 0;
    int igs = ngs,
        ires = nres;
    int prev_p = 1;
    unsigned wtp = 0;

    if (ngs == 0 && nres == 0) {
        wtp = 1;
        prev_p = 0;
    }

    while (igs != 0 || ires != 0) {
        if (igs == 0) { // we have additional stuff
            c->bfp += 1;
            prev_p = 0;
            res -= 1;
        } else if (ires == 0) { // we missed some stuff
            c->bfn += 1;
            prev_p = 0;
            gs -= 1;
        } else {
            if(gs->bound[igs] == res->bound[ires]) {
                c->btp += 1;
                if (prev_p) {
                    wtp += 1;
                }
                prev_p = 1;
                igs -= 1;
                ires -= 1;
            } else if (gs->bound[igs] > res->bound[ires]) {
                c->bfn += 1;
                prev_p = 0;
                igs -= 1;
            } else if (gs->bound[igs] < res->bound[ires]) {
                c->bfp += 1;
                prev_p = 0;
                ires -= 1;
            }
        }
    }

    if(prev_p) {
        wtp += 1;
    }
    c->wfp += nres + 1 - wtp;
    c->wfn += ngs + 1 - wtp;
    c->wtp += wtp;

}

void trie_insert_seg(struct trie *t, struct unitseq *seq, 
        struct segmentation *seg)
{

    size_t i, first, last;

    first = 0;
    for (i = 0; seg != NULL && i < seg->len; i++) {
        last = seg->bound[i];
        trie_insert(t, seq->seq + first,  last - first);
        first = last;
    }
    trie_insert(t, seq->seq + first, seq->len - first);
}

void print_prf(struct input *in, struct segmentation **out, 
        size_t range_start, size_t range_end, uint8_t options)
{
    struct seg_score sc = { .bp=0.0, .br=0.0, 
                            .wp=0.0, .wr=0.0,
                            .lp=0.0, .lr=0.0
                          };
    int i;
    struct seg_counts c = { 0 };
    size_t  bcount = 0,     // number of boundaries
            nbcount = 0;    // number of `non-boundaries'
    struct trie *gs_lex = trie_init(in->sigma_len + 1),
                *model_lex = trie_init(in->sigma_len + 1);

    for (i = range_start; i < range_end; i++) {
        struct segmentation *seg = out[i];
        struct segmentation *gs_seg = in->u[i]->gs_seg;
        struct unitseq *seq = in->u[i]->phon; // TODO: syllable segmentation
        size_t len = seq->len; 

        bcount +=  (gs_seg != NULL) ? gs_seg->len : 0;
        nbcount += len - 1 -  (gs_seg != NULL) ? gs_seg->len : 0;
        
        get_tp_fn_fp(&c, gs_seg, seg);

        trie_insert_seg(gs_lex, seq, gs_seg);
        trie_insert_seg(model_lex, seq, seg);

        if (options & SCORE_EDGES) {
            c.btp += 1;
        }

    }

    sc.bp = (double)c.btp / (double)(c.btp + c.bfp);
    sc.br = (double)c.btp / (double)(c.btp + c.bfn);

    sc.wp = (double)c.wtp / (double)(c.wtp + c.wfp);
    sc.wr = (double)c.wtp / (double)(c.wtp + c.wfn);

    struct trie_iter *ti = trie_iter_init(model_lex);
    segunit_t *tmp;
    size_t  tmplen;
    struct trie_node *node_m = NULL, 
                     *node_gs = NULL;
    while ((node_m = trie_iter_next(ti, &tmp, &tmplen))) {
        if (node_m->count_final == 0) // not a full word, skip
            continue;
        node_gs = trie_lookup(gs_lex, tmp, tmplen);
        if (node_gs == NULL && node_gs->count_final != 0) {
            c.ltp += 1;
        } else {
            c.lfp += 1;
        }
    }
    trie_iter_free(ti);

    c.lfn = gs_lex->types - c.ltp;

/*
    GHashTableIter iter;
    g_hash_table_iter_init (&iter, lex_out);
    gpointer key, val;
    while (g_hash_table_iter_next (&iter, &key, &val)) {
        if(g_hash_table_lookup(lex_in, key)) {
            ++c.ltp;
        } else {
            ++c.lfp;
        }
    }
    c.lfn = g_hash_table_size (lex_in) - c.ltp;
*/

    sc.lp = (double)c.ltp / (double)(c.ltp + c.lfp);
    sc.lr = (double)c.ltp / (double)(c.ltp + c.lfn);

    sc.eo = (double) c.bfp  / (double) nbcount;
    sc.eu = (double) c.bfn  / (double) bcount;

    memcpy(&sc.c, &c, sizeof c);

    g_hash_table_destroy(lex_in);
    g_hash_table_destroy(lex_out);

    char *sep = (options & SCORE_OPT_LATEX) ? "& " : ",";
    char *eol = (options & SCORE_OPT_LATEX) ? "\\\\\\hline" : "";
    if (options & SCORE_OPT_HEADER)  {
        printf("start%1$send%1$s btp%1$sbfp%1$sbfn%1$swtp%1$swfp%1$swfn%1$sltp%1$slfp%1$slfn%1$s ", sep);
        printf("bp%1$sbr%1$sbf%1$swp%1$swr%1$swf%1$slp%1$slr%1$slf%1$s eo%1$seu%2$s\n", sep, eol);
    }
    printf("%2$zu%1$s%3$zu%1$s "
           "%4$u%1$s%5$u%1$s%6$u%1$s"
           "%7$u%1$s%8$u%1$s%9$u%1$s"
           "%10$u%1$s%11$u%1$s%12$u%1$s ", 
            sep, range_start, range_end,
            sc.c.btp, sc.c.bfp, sc.c.bfn,
            sc.c.wtp, sc.c.wfp, sc.c.wfn,
            sc.c.ltp, sc.c.lfp, sc.c.lfn);
    printf("%2$f%1$s%3$f%1$s%4$f%1$s"
           "%5$f%1$s%6$f%1$s%7$f%1$s"
           "%8$f%1$s%9$f%1$s%10$f%1$s"
           "%11$f%1$s%12$f%13$s\n",
            sep,
            sc.bp, sc.br, f_score(sc.bp,sc.br), 
            sc.wp, sc.wr, f_score(sc.wp,sc.wr),
            sc.lp, sc.lr, f_score(sc.lp,sc.lr),
            sc.eo, sc.eu,
            eol);
}
