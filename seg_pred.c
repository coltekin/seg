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
 * Segmentation using (only) predictability measure(s)
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "cmdline.h"
#include "io.h"
#include "phonstats.h"
#include "seg_pred.h"
#include "predictability.h"
#include "print.h"
#include "prob_dist.h"

static struct phonstats *ps;
static int xmin, xmax;
static int ymin, ymax;
static unsigned mmask = 0;
static unsigned nvotes = 0;
static struct prob_dist **mdist = NULL;
static double *mweight;     // weights of measures (or measure-context pair)
static size_t *merr = NULL; // count of errors made by a particular measure
static size_t bc_count;     // count of all boundary candidates

#define SIGN(x) ( ((x) > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0) )
// #define SIGN(x) ( ((x) > 0.0) ? 1.0 :  -1.0 )
#define LOGISTIC(x) ( 1.0 / (1.0 + exp(-(x))) )

static double
get_vote_peak(unsigned mm, int  pt, double prev, double curr, double next)
{
    double left  = curr - prev;
    double right = curr - next;
    double peak;

    assert(pt != peak_arg_dual);

    if (mm & (p_info[PM_JP].mmask |
              p_info[PM_TP].mmask |
              p_info[PM_MI].mmask |
              p_info[PM_TPR].mmask)){
        right = -right;
        left  = -left;
    }

    switch (pt) {
        case peak_arg_strict: 
            if(SIGN(left) != SIGN(right)) peak = 0.0;
            else peak = left + right;
        break;
        case peak_arg_strict2: 
            assert(opt.pred_norm_flag);
            if(SIGN(left) != SIGN(right)) peak = 0.0;
            else if(curr < 0.0) peak = 0.0;
            else peak = left + right;
        break;
        case peak_arg_relaxed:
            peak = left + right;
        break;
        case peak_arg_right:
            peak = right;
        break;
        case peak_arg_left:
            peak = left;
        break;
    }

    switch (opt.vote_arg) {
        case vote_arg_binary: 
            return (peak > 0.0) ? 1 : -1; 
        break;
        case vote_arg_diff: 
            return peak; 
        break;
        case vote_arg_lgdiff: 
            return 2 * LOGISTIC(peak) - 1;
        break;
        default: 
            fprintf(stderr, "unknown vote arg %d\n", opt.vote_arg);
            exit (-1);
    }
}

void 
segment_pred_init(struct input *in)
{
    int i;

    for (i = 0; i < ((opt.pred_m_given) ? opt.pred_m_given : 1) ; ++i) {
       mmask |= p_info[opt.pred_m_arg[i]].mmask;
    }

    xmin = opt.pred_xmin_arg;
    ymin = opt.pred_ymin_arg;
    xmax = opt.pred_xmax_arg;
    ymax = opt.pred_ymax_arg;

    if(opt.pred_xlen_given) {
        xmin = xmax = opt.pred_xlen_arg;
    }
    if(opt.pred_ylen_given) {
        ymin = ymax = opt.pred_ylen_arg;
    }

    nvotes = (xmax - xmin + 1) * (ymax - ymin + 1)
            * __builtin_popcount(mmask)
            * ((opt.peak_arg == peak_arg_dual) ? 2 : 1);

    if(opt.pred_norm_given) {
        mdist = malloc(nvotes * sizeof(*mdist));
        for (i = 0; i < nvotes; i++) {
            mdist[i] = prob_dist_new();
        }
    }

    mweight = malloc(nvotes * sizeof (*mweight));
    if(opt.combine_arg == combine_arg_wmv) {
        merr = malloc(nvotes * sizeof (*merr));
    }
    for (i = 0; i < nvotes; i++) {
        mweight[i] = 1.0;
        if(opt.combine_arg == combine_arg_wmv) {
            merr[i] = 0;
        }
    }
    bc_count = 0;

    ps = phonstats_new(xmax + ymax, NULL);
    if (opt.prior_data_given) {
        struct input *prior;
        if (!strcmp(opt.prior_data_arg, "input")) {
            prior = read_input(opt.prior_data_arg);
        } else {
            prior = in;
        }
        for (i = 0; i < prior->size; i++) {
            int m;
            phonstats_update(ps, prior->u[i].s);
            if(opt.pred_norm_given) {
                struct mlist_list *ml = pred_mlist_list(ps, prior->u[i].s, 
                                                        mmask, xmin, xmax, 
                                                        ymin, ymax);
                for (m = 0; m < ml->llen; m++) {
                    int j;
                    for (j = 1; j < strlen(prior->u[i].s); j++) {
                        prob_dist_update(mdist[m], ml->mlist[m][j]);
                    }
                }
                pred_mlist_list_free(ml);
            }
        }
        if (prior != in) {
            input_free(prior);
        }
    }

    if (opt.pred_printoptions_flag) {
        printf("measure=");
        for (i = 0; i < ((opt.pred_m_given) ? opt.pred_m_given : 1) ; ++i) {
           printf("%s,", p_info[opt.pred_m_arg[i]].sname);
        }
        printf(";combine=");
        switch (opt.combine_arg) {
            case combine_arg_mv:  printf("mv;"); break;
            case combine_arg_any: printf("any;"); break;
            case combine_arg_all: printf("all;"); break;
            case combine_arg_wmv: printf("wmv;"); break;
            default: printf("wmv;"); break;
        } 
        printf("peak=");
        switch (opt.peak_arg) {
            case peak_arg_strict:  printf("strict;"); break;
            case peak_arg_strict2:  printf("strict2;"); break;
            case peak_arg_right:   printf("rithg;"); break;
            case peak_arg_left:    printf("left;"); break;
            case peak_arg_relaxed: printf("relaxed;"); break;
            case peak_arg_lr:    printf("lr;"); break;
            case peak_arg_dual:    printf("dual;"); break;
            default:               printf("dual;"); break;
        } 
        printf("vote=");
        switch (opt.vote_arg) {
            case vote_arg_diff:   printf("diff;"); break;
            case vote_arg_lgdiff: printf("lgdiff;"); break;
            case vote_arg_binary: printf("binary;"); break;
            default:              printf("binary;"); break;
        } 
        printf("norm=%d;", (opt.pred_norm_flag));
        printf("xmin=%d;", xmin);
        printf("ymin=%d;", xmin);
        printf("xmax=%d;", xmax);
        printf("ymax=%d;", ymax);
        printf("prior=%s;", (opt.prior_data_given)? opt.prior_data_arg : "none");
        printf("\n");
    }
}


// in-place normalization of a pred_list_list
inline void
normalize_pred_list(struct mlist_list *ml, int ulen)
{
    int i, j;

    assert(opt.pred_norm_given);

    for (i = 0; i < ml->llen; i++) {
        for (j = 0; j < ulen; j++) {
            ml->mlist[i][j] = prob_dist_norm(mdist[i], ml->mlist[i][j]);
        }
    }
}

struct seglist * 
segment_pred(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    struct seglist *segl = seglist_new();
    int  len = strlen(u) - 1;
    unsigned short seg[len + 1];
    struct mlist_list *ml = NULL;
    int j, i = 1, k;

    seg[0] = 0;

    phonstats_update(ps, u);

    ml = pred_mlist_list(ps, u, mmask, xmin, xmax, ymin, ymax);

    if (opt.pred_norm_given) {
        for (k = 0; k < ml->llen; k++) {
            for (j = 0; j <= len; j++) {
                prob_dist_update(mdist[k], ml->mlist[k][j]);
            }
        }
        normalize_pred_list(ml, len);
    }

//print_pred_list(u, ml->mlist[0]);

    for (j = 1; j <= len; j++) {
        double votec = 0.0;
        int    dual = (opt.peak_arg == peak_arg_dual) + 1;
        double vote[ml->llen * dual];

        for (k = 0; k < ml->llen; k++) {
            int d;
            enum pred_measure m = ml->m[k];
            double *mlist = ml->mlist[k];
            for (d = 1; d <= dual; d++) {
                vote[k*d] = (dual == 1) ?
                            get_vote_peak(p_info[m].mmask, 
                                          opt.peak_arg, 
                                          mlist[j - 1], mlist[j], mlist[j + 1])
                          : get_vote_peak(p_info[m].mmask, 
                                          (d == 1) ? peak_arg_left : peak_arg_right, 
                                          mlist[j - 1], mlist[j], mlist[j + 1]);
                votec += mweight[k*d] * vote[k*d];
//printf("[%d/%d]: %0.2f-%0.2f-%0.2f:  vote = %f\n", j, k*d, mlist[j - 1], mlist[j], mlist[j + 1], vote[k]);
            }
        }
        switch (opt.combine_arg) {
            case combine_arg_wmv:
            case combine_arg_mv:
                if (votec > 0.0) { // insert a boundary
                    seg[i] = j;
                    ++i; ++seg[0];
                }
            break;
            case combine_arg_any:
                if (votec > -(dual * ml->llen)) {
                    seg[i] = j;
                    ++i; ++seg[0];
                }
            break;
            case combine_arg_all:
                if (votec == dual * ml->llen) {
                    seg[i] = j;
                    ++i; ++seg[0];
                }
            break;
            default:
                fprintf(stderr, "unknown combination method %d\n", opt.combine_arg);
        }

        ++bc_count;
        if (opt.combine_arg == combine_arg_wmv) { // adjust the weights
            for (k = 0; k < ml->llen; k++) {
                if (SIGN(vote[k]) != SIGN(votec)) {
                    ++merr[k];
                }
                double errr = ((double)merr[k] / (double) bc_count);
                if (errr > 0.5) errr = 0.5;
                mweight[k] = 2 * (0.5 - errr);
            }
        }
    }
    seglist_add(segl, seg);

    pred_mlist_list_free(ml);
    ml = NULL;
    return segl;
}

void 
segment_pred_update(char *s, struct seglist *segl)
{
//    phonstats_dump(ps);

/*
    char **segstr;
    char **seg;
    assert(segl->nsegs == 1);

    segstr = seg_to_strlist(s, segl->segs[0]);
    seg = segstr;
    while (*seg) {
        cg_lexicon_add(L, *seg, "C", NULL);
        phonstats_update(ps, *seg);
        ++seg;
    }
    free_strlist(segstr);
*/
}

void 
segment_pred_cleanup()
{
    int i, x, y, m;
    // TODO: there is more to cleanup
    if(opt.pred_norm_given) {
        for (i = 0; i < nvotes; i++) {
            prob_dist_free(mdist[i]);
        }
        free(mdist); mdist = NULL;
    }

    if (opt.pred_printw_flag && opt.combine_arg == combine_arg_wmv) {
        i = 0;
        for(x = xmin; x <= xmax; x++) {
            for(y = ymin; y <= ymax; y++) {
                for(m = 0; m < PM_MAX; m++) { 
                    if(p_info[m].mmask & mmask) {
                        fprintf(stderr, "%s/%d/%d: merr = %zu,  mweight = %f\n", 
                                p_info[m].sname, x, y, merr[i], mweight[i]);
                        ++i;
                    }
                }
            }
        }
    }

    free(mweight); mweight = NULL;
    if (merr) free(merr); merr = NULL;

    return;
}
