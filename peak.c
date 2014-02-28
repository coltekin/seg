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
#include "peak.h"

double
get_vote_peak(double prev, double curr, double next, 
              int peak_type,
              struct mdata *md)
{
    int mdir = md->info->dir;
    double left  = mdir * (curr - prev);
    double right = mdir * (curr - next);
    double peak;

    if (peak_type == peak_arg_dual) peak_type = peak_arg_left;
    if (peak_type == peak_arg_lr)  {
        peak_type = (md->info->lr == -1) ? peak_arg_right : peak_arg_left;
    }

    switch (peak_type) {
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
            return (peak > 0.0) ? 1.0 : -1.0; 
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

int
get_votes_peak(double ***vote_l, double ***vote_r, struct mlist *ml)
{
    int m, i;
    int dual = (opt.peak_arg == peak_arg_dual);

    double **vl = NULL, **vr = NULL;
    
    if (*vote_l == NULL) {
        assert (*vote_r == NULL);
        vl = malloc(ml->len * sizeof (double *));
        if (dual) {
            vr = malloc (ml->len * sizeof (double *));
        }
        for (m = 0; m < ml->len; m++) {
            vl[m] = malloc (ml->slen * sizeof (double));
            if (dual) {
                vr[m] = malloc (ml->slen * sizeof (double));
            }
        }
    }

    for (m = 0; m < ml->len; m++) {
        struct mdata *md = ml->m[m];
        double *mval = ml->mlist[m];
        for (i = 1; i < ml->slen; i++) {
            vl[m][i] = get_vote_peak(mval[i - 1], mval[i], mval[i + 1],
                                         opt.peak_arg, md);
            if (dual) {
                vr[m][i] = get_vote_peak(mval[i - 1], mval[i], mval[i + 1],
                                             peak_arg_right, md);
            }
        }
    }

    *vote_l = vl;
    *vote_r = (dual) ? vr : NULL;
    return (dual) ? 2 : 1;
}
