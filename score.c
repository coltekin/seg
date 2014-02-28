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
#include "options.h"
#include "seglist.h"


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

void
get_tp_fn_fp(struct seg_counts *c, char *u,
             unsigned short *gs, unsigned short *res)
{
    int ngs = (gs != NULL) ? gs[0] : 0, 
        nres = (res != NULL) ? res[0] : 0;
    int igs = ngs,
        ires = nres;
    int prev_p = 1;
    unsigned wtp = 0;
    char **segstr;

    if (ngs == 0 && nres == 0) {
        wtp = 1;
        prev_p = 0;
    }
    while (igs != 0 || ires != 0) {
        if (igs == 0){// we have additional stuff
            ++c->bfp;
            prev_p = 0;
            --ires;
        } else if (ires == 0){// we missed some stuff
            ++c->bfn;
            prev_p = 0;
            --igs;
        } else {
            if(gs[igs] == res[ires]) {
                ++c->btp;
                if (prev_p) {
                    ++wtp;
                }
                prev_p = 1;
                --igs;
                --ires;
            } else if (gs[igs] > res[ires]) {
                ++c->bfn;
                prev_p = 0;
                --igs;
            } else if (gs[igs] < res[ires]) {
                ++c->bfp;
                prev_p = 0;
                --ires;
            }
        }
    }

    if(prev_p) {
        ++wtp;
    }
    c->wfp += nres + 1 - wtp;
    c->wfn += ngs + 1 - wtp;
    c->wtp += wtp;

    segstr = seg_to_strlist(u, gs);
    char **tmp = segstr;
    while (*tmp){
        if (g_hash_table_lookup(lex_in, *tmp)) {
            free(*tmp);
        } else {
            g_hash_table_insert (lex_in, *tmp, *tmp);
        }
        ++tmp;
    }
    free(segstr);

    segstr = seg_to_strlist(u, res);
    tmp = segstr;
    while (*tmp){
        if (g_hash_table_lookup(lex_out, *tmp)) {
            free(*tmp);
        } else {
            g_hash_table_insert (lex_out, *tmp, *tmp);
        }
        ++tmp;
    }
    free(segstr);
}

void print_prf(struct input *in, struct output *out, size_t offset, short print_header)
{
    struct seg_score sc = { .bp=0.0, .br=0.0, 
                            .wp=0.0, .wr=0.0,
                            .lp=0.0, .lr=0.0
                          };
    int i;
    struct seg_counts c = { 0 };
    size_t  bcount = 0, nbcount = 0;

    assert(offset < out->size);

    lex_in = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
    lex_out = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);

    if (opt.score_arg == score_arg_random) {
        srand((unsigned int)time(NULL));
    }


    for (i = offset; i < out->size; i++) {
        assert(out->u != NULL);
        assert(in->u != NULL);
        assert(0 == strcmp(in->u[i].s,out->u[i].s));
        unsigned short *seg = NULL;
        unsigned short *tmp = NULL;
        int len = strlen(in->u[i].s);
        
        if (out->u[i].segl->nsegs != 0) {
            switch (opt.score_arg) {
                case score_arg_random: {
                    int k = rand() / (RAND_MAX / out->u[i].segl->nsegs + 1);
                    seg = out->u[i].segl->segs[k];
                } break;
                case score_arg_first: {
                    seg = out->u[i].segl->segs[0];
                } break;
                case score_arg_best: {
                    int k, best = 0;
                    double best_score = 0.0;
                    for (k = 0; k < out->u[i].segl->nsegs; k++) {
                        if (out->u[i].segl->score[k] > best_score) {
                            best_score = out->u[i].segl->score[k];
                            best = k;
                        }
                    }
                    seg = out->u[i].segl->segs[best];
                } break;
                case score_arg_any: {
                    int j, k;
                    tmp = malloc ((len + 1) * sizeof (*tmp));
                    tmp[0] = 0;
                    for (j = 1; j < len; j++) {
                        for (k = 0; k < out->u[i].segl->nsegs; k++) {
                            if (seg_check(out->u[i].segl->segs[k], j)){
                                ++tmp[0];
                                tmp[tmp[0]] = j;
                                break;
                            }
                        }
                    }
                    seg = tmp;
                } break;
                default:
                    assert(0 && "Unknown score option");
            }
        }
        
        get_tp_fn_fp(&c, in->u[i].s, in->u[i].seg, seg);

        int btmp = (in->u[i].seg != NULL) ? in->u[i].seg[0] : 0;
        bcount +=  btmp;

        nbcount += len - 1 - btmp;

/*        printf("%s: ", in->u[i].s);
        print_intarray_fp(stdout, in->u[i].seg);
        print_intarray_fp(stdout, seg);
        printf("slen = %d, bcount = %d, nbcount = %d c.bfp = %zu\n", len, btmp, len - 1 - btmp, c.bfp);
*/

        if (opt.score_edges_flag) {
            c.btp += 1;
        }

        if (tmp != NULL) {
            free(tmp);
            tmp = NULL;
        }
    }

    sc.bp = (double)c.btp / (double)(c.btp + c.bfp);
    sc.br = (double)c.btp / (double)(c.btp + c.bfn);

    sc.wp = (double)c.wtp / (double)(c.wtp + c.wfp);
    sc.wr = (double)c.wtp / (double)(c.wtp + c.wfn);

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

/*
PINFO("h/m/f:%zu,%zu,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", offset, out->size,
                                                    c.btp, c.bfn, c.bfp, 
                                                    c.wtp, c.wfn, c.wfp,
                                                    c.ltp, c.lfn, c.lfp);
*/
    sc.lp = (double)c.ltp / (double)(c.ltp + c.lfp);
    sc.lr = (double)c.ltp / (double)(c.ltp + c.lfn);

    sc.eo = (double) c.bfp  / (double) nbcount;
    sc.eu = (double) c.bfn  / (double) bcount;

    memcpy(&sc.c, &c, sizeof c);

    g_hash_table_destroy(lex_in);
    g_hash_table_destroy(lex_out);

    char *sep = (opt.print_latex_flag) ? "& " : ",";
    char *eol = (opt.print_latex_flag) ? "\\\\\\hline" : "";
    if (print_header)  {
        printf("start%1$send%1$s btp%1$sbfp%1$sbfn%1$swtp%1$swfp%1$swfn%1$sltp%1$slfp%1$slfn%1$s ", sep);
        printf("bp%1$sbr%1$sbf%1$swp%1$swr%1$swf%1$slp%1$slr%1$slf%1$s eo%1$seu%2$s\n", sep, eol);
    }
    printf("%2$zu%1$s%3$zu%1$s "
           "%4$u%1$s%5$u%1$s%6$u%1$s"
           "%7$u%1$s%8$u%1$s%9$u%1$s"
           "%10$u%1$s%11$u%1$s%12$u%1$s ", 
            sep, offset, out->size,
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
