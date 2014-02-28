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
 * segparse.c
 * 
 * This file contains modified versions of some parsing functions 
 * that are found in ../cgparse/packed_chart.c and 
 * ../cgparse/cyk_packed.c.
 *
 * main difference is that the input is a unsegmented string. 
 * during parsing, we also search the related spans that are 
 * in the lexicon.
 *
 * The implementation here uses a lot from the CG parser, 
 * that causes some reduplication of code.
 *
 * FIXME: this duplicates quite a lot of code from original 
 *       parsing code.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "segparse.h"
#include "stack.h"
#include "strutils.h"

inline void
print_intarray_fp(FILE *fp, short *a)
{
    int j;

    if(a == NULL){
        fprintf(fp, "<>\n");
        return;
    }
    fprintf(fp, "<");
    for (j = 0; j <= a[0]; j++) {
        fprintf(fp, "%d,", a[j]);
    }
    fprintf(fp, "\b>\n");
}


cg_cat *seg_combined_cat = NULL; // cat to return when combining two elements

/*
 * seg_combine(cg_cat *L, cg_cat *R)
 * 
 * returns `seg_combined_cat' regardless of the input categories.
 * 
 * 
 */
cg_cat *
seg_combine(cg_cat *L, cg_cat *R)
{
    if(seg_combined_cat == NULL)  {
        PFATAL("seg_combined_cat is not initialized\n");
    }
    return seg_combined_cat;
}

/* 
 * seg_parse() -- a modified version of cyk parser
 *
 * input is an unsegmented string.
 * 
 * the function argument `combine' allows different combination 
 * functions to be used.
 *
 * output is a packed chart.
 */
struct chart *
seg_parse(cg_lexicon *l, char *input, combine_funct_t combine)
{
    unsigned short i, j, k;
    size_t         N = strlen(input);

    struct chart *chart = chart_new(N);

    for (j=0; j < N; j++){
        char *sp = str_span(input, j, 1);
        chart->input[j] = sp;
    }

    if (combine == seg_combine) {
        seg_combined_cat = cg_lexicon_addcat(l, "C");
    }

    for(i=0; i <= N; i++) {
        for(j=0; j < (N - i ); j++){
            char *sp = str_span(input, j, i+1);
            cg_lexilist *ll = cg_lexicon_lookup(l, sp);

            while(ll) {
                chart_node_add(chart, i, j, ll->lexi->cat, NULL, NULL);
                ll = ll->next_hom;
            }
            free(sp);

            for(k=1; k <= i; k++){
                struct chart_node *nodeL, *nodeR;
                cg_cat *catL, *catR;
                unsigned short iL, iR, jL, jR; // indx for l/r consitituents 

                iL = k - 1; jL = j;
                iR = i - k; jR = j + k;
                
                /* search if any of the L,R pairs combine */
                
                nodeL = chart->node[iL][jL];
                while(nodeL != NULL) {
                    catL = nodeL->cat;
                    nodeR = chart->node[iR][jR];
                    while(nodeR != NULL) {
                        cg_cat  *res;
                        catR = nodeR->cat;
                        res = combine(catL, catR);
                        if(res){
                            chart_node_add(chart, i, j, res, nodeL, nodeR);
                        }
                        nodeR = nodeR->next;
                    }
                    nodeL = nodeL->next;
                }

            }
        }
    }
    return chart;
}

struct seglist *
get_segs_full(struct chart *chart)
{
    struct stack    *st = stack_init();
    int     i = 0,  // span length - 1
            j = 0;  // span start
    unsigned short     *s = NULL;
    struct seglist  *segs = seglist_new();

    if(chart->node[chart->size - 1][0] == NULL) {
        seglist_add(segs,NULL);
        return segs;    //no segment starting at 0
    }
    s = malloc ((chart->size + 1) * sizeof (*s));     // current seglist
    s[0] = 0;

    do {
        for (i = 0; i < (chart->size - j); i++) {
            unsigned char found = 0;
            struct chart_node *n = chart->node[i][j];
            while (n) {
                if (n->back == NULL) { // terminal node
                    found = 1;
                    break;
                }
                n = n->next;
            }

            if (found) { // there is a terminal node at <i, j>
                unsigned short *new = malloc(sizeof (*new) * (chart->size + 1));
                memcpy(new, s, sizeof (*new) * (chart->size + 1));
                if (new[0] == 0) {
                    new[new[0] + 1] = i + 1;
                } else {
                    new[new[0] + 1] = new[new[0]] + i + 1;
                }
                ++new[0];
                if(new[new[0]] == chart->size) { // complete segm.
                    --new[0];
                    seglist_add(segs, new);
                    free(new);
                } else {
                    stack_push(st, new);
                }
            }
        }
        free(s);
        s = stack_pop(st);
        if (s) {
            j = s[s[0]];
        }
    } while (s);

    stack_free(st, 0);

    return segs;
}

inline short *
segtmp_dup(short *s, int len)
{
    short *new = malloc(sizeof (*new) * len);
    memcpy(new, s, sizeof (*new) * len);
    return new;
}

struct seglist *
get_segs_partial_opt(struct chart *chart, enum segparse_opt o)
{
    struct stack    *st = stack_init();
    int     i = 0,  // span length - 1
            j = 0;  // span start
    short     *s = NULL;
    struct seglist  *segs = seglist_new();

    s = malloc ((chart->size + 1) * sizeof (*s));     // current seglist

    s[0] = 0;

    do {
        unsigned char found_j = 0;
        for (i = 0; i < (chart->size - j - 2); i++) {
            unsigned char found_i = 0;
            struct chart_node *n = chart->node[i][j];
            while (n) {
                if (n->back == NULL) { // terminal node
                    found_i = 1;
                    found_j = 1;
                    break;
                }
                n = n->next;
            }

            if (found_i) { // there is a terminal node at <i, j>
                short *new = segtmp_dup(s, (chart->size +1));
                new[new[0] + 1] = ABS(new[new[0]]) + i + 1;
                ++new[0];
                stack_push(st, new);
            }
//fprintf(stderr, "xx: chart->size=%d j=%d i=%d f_i=%d f_j=%d\n", chart->size, j, i, found_i, found_j);
        }

        short *new = NULL;
        if (!found_j) { // no starting at j
            if (s[s[0]] < chart->size){ 
                if ((s[s[0]] < 0)) { // we did not have on previous attempt
                    new = segtmp_dup(s, (chart->size +1));
                    --new[new[0]];
//fprintf(stderr, "\tba s[0]=%d new[0]=%d / s[s[0]]=%d new[new[0]]=%d\n", s[0], new[0], s[s[0]], new[new[0]]);
                    stack_push(st, new);
                } else {
//fprintf(stderr, "\tbu\n");
                    new = segtmp_dup(s, (chart->size +1));
                    new[new[0] + 1] = -new[new[0]] - 1;
                    ++new[0];
                    stack_push(st, new);
                }
            } else if (s[s[0]] == chart->size) { 
//fprintf(stderr, "\tgu\n");
                new = segtmp_dup(s, (chart->size +1));
                stack_push(st, new);
            }
        }
//fprintf(stderr, "\t!f_j: s=");print_intarray_fp(stderr, s);
//fprintf(stderr, "\t\t new=");print_intarray_fp(stderr, new);

        free(s);

        s = stack_pop(st);
        while (s && (ABS(s[s[0]]) == chart->size)) {
            switch (o) {
                case SPOPT_ONE: {
                    int count = 0;
                    for (i = 1; i <= s[0]; i++) {
                        if (s[i] < 0) {
                            count++;
                            if (count > 1) {
                                break;
                            }
                        }
                    }
                    if(count <= 1) {
                        --s[0];
                        seglist_add_signed(segs, s);
                    }
                } break;
                case SPOPT_END: {
                    int count = 0;
                    for (i = 1; i < s[0]; i++) {
                        if (s[i] < 0) {
                            count++;
                            break;
                        }
                    }
                    if(count == 0) {
                        --s[0];
                        seglist_add_signed(segs, s);
                    }
                } break;
                case SPOPT_BEGIN: {
                    int count = 0;
                    for (i = 2; i <= s[0]; i++) {
                        if (s[i] < 0) {
                            count++;
                            break;
                        }
                    }
                    if(count == 0) {
                        --s[0];
                        seglist_add_signed(segs, s);
                    }
                } break;
                case SPOPT_BEGINEND: {
                    int count = 0;
                    for (i = 2; i < s[0]; i++) {
                        if (s[i] < 0) {
                            count++;
                            break;
                        }
                    }
                    if(count == 0) {
                        --s[0];
                        seglist_add_signed(segs, s);
                    }
                } break;
                default: {
                    --s[0];
                    seglist_add_signed(segs, s);
                } break;
            }
            
            free(s);
            s = stack_pop(st);
        }
        if (s) {
            j = ABS(s[s[0]]);
        }
    } while (s);

    stack_free(st, 0);

    return segs;
}

struct seglist *
get_segs_partial(struct chart *chart)
{
    return get_segs_partial_opt(chart, SPOPT_ALL);
}

#define SEP_STR "-"
void
write_segs(FILE *fp, struct chart *c)
{
    struct seglist  *segs = get_segs_full(c);
    int     i, j;

    if(segs == NULL || segs->nsegs == 0) {
        printf("No full segments.\n");
//        return;
    }

/*

    for (i=0; i < segs->nsegs; i++) {
        int *seg = segs->segs[i];
        int             pos = 0;
        for (j = 1; j <= seg[0]; j++) {
            for(k = pos; k < seg[j]; k++) {
                printf("%s", c->input[k]);
            }
            pos = seg[j];
            if (pos != 0 && j != seg[0]) 
                printf("%s", SEP_STR);
        }
        printf("\t");
    }
    printf("\n");
    seglist_free(segs);
*/

    segs = get_segs_partial(c);

    for (i=0; i < c->size; i++) {
        printf("%s", c->input[i]);
    }
    printf(" [%d]: ", segs->nsegs);
    for (i = 0; i < segs->nsegs; i++) {
        printf("<");
        for (j = 0; j <= segs->segs[i][0]; j++) {
            printf("%d,", segs->segs[i][j]);
        }
        printf("\b> ");
    }
    printf("\n");
    seglist_free(segs);
}
