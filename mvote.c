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
#include "options.h"
#include "mvote.h"
#include "peak.h"
#include "mdata.h"

static int nvotes = -1;

void mv_init()
{
    nvotes = 0;
}

double 
weight_update(double vote, double mvote, double cweight)
{
    double diff = (SIGN(mvote) == SIGN(vote)) ? 1.0 : -1.0;
    double tpcount = (double) (nvotes - 1)  * cweight + diff;

    // this prevents sign of the weights to flip when nvotes = 1 & diff = -1
    if (tpcount < 0) tpcount = 0.5; 

    return tpcount / (double) nvotes;
}


double *
mv_getvotes(double *mv, struct mlist *ml) 
{
    int i, m;
    if (ml->slen == 0) ml->slen = strlen(ml->s);

    assert(nvotes >= 0);
    assert(ml->len != 0);

    double **vote_l = NULL;
    double **vote_r = NULL;

    int vc = 0;
    switch (opt.boundary_method_arg) {
    case boundary_method_arg_peak:
        vc = get_votes_peak(&vote_l, &vote_r, ml);
    break;
//    case boundary_method_arg_threshold:
//        vc = get_votes_threshold(&vote_l, &vote_r, ml);
//    break;
    default:
        assert("we should not be here!");
    }

    if (mv == NULL) {
        mv = malloc (ml->slen * sizeof (*mv));
    }

    for (i = 1; i < ml->slen; i++) {
        int votec = 0;

        mv[i] = 0.0;
        ++nvotes;
        for (m = 0; m < ml->len; m++) {
            mv[i] += ml->m[m]->w_l * vote_l[m][i];
            if (vote_l[m][i] > 0.0) votec++;
            if (vc == 2 ) { // we have votes for both left and right
                mv[i] += ml->m[m]->w_r * vote_r[m][i];
                if (vote_r[m][i] > 0.0) votec++;
            }
        }

        switch (opt.combine_arg) {
            case combine_arg_mv:
                mv[i] = (mv[i] + (double) ml->len) / 2 
                          - opt.combine_rate_arg * (double) ml->len;
            break;
            case combine_arg_any:
                mv[i] = (votec > 0) ? 1 : -1;
            break; 
            case combine_arg_all:
                mv[i] = (votec == ml->len) ? 1 : -1;
            break; 
            case combine_arg_wmv: 
            default: {
                double mvtmp =  (mv[i] + (double) ml->len) / 2 
                               - opt.combine_rate_arg * (double) ml->len;
                for (m = 0; m < ml->len; m++) {
                    ml->m[m]->w_l = weight_update(vote_l[m][i], mvtmp, 
                                                            ml->m[m]->w_l);
                    if (opt.peak_arg == peak_arg_dual) {
                        ml->m[m]->w_r = weight_update(vote_r[m][i], mvtmp, 
                                                                ml->m[m]->w_r);
                    } else {
                        ml->m[m]->w_r = ml->m[m]->w_l;
                    }
                }
                mv[i] = mvtmp;
            } break;
        }
    }


    if (vote_l) {
        for (m = 0; m < ml->len; m++) {
            free(vote_l[m]);
        }
        free(vote_l);
    }
    if (vote_r) {
        for (m = 0; m < ml->len; m++) {
            free(vote_r[m]);
        }
        free(vote_r);
    }
    return mv;
}
