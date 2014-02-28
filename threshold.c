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
#include <stdio.h>
#include <assert.h>
#include "measures.h"
#include "threshold.h"

static short init = 0;

void
threshold_init()
{
    if (!opt.threshold_given) {
        fprintf(stderr, "Warning: no threshold value given!\n");
    }
    if (!opt.norm_given) {
        fprintf(stderr, "Warning: using threshold without normalization!\n");
    }
    init = 1;
}

#define normalize(x, y) y

int
get_votes_treshold(double ***vote_l, double ***vote_r, struct mlist *ml)
{
    int m, i;
    double **vl = NULL;
    
    assert (init == 1);
    assert (*vote_r == NULL);

    if (*vote_l == NULL) {
        vl = malloc(ml->len * sizeof (double *));
        for (m = 0; m < ml->len; m++) {
            vl[m] = malloc (ml->slen * sizeof (double));
        }
    }

    for (m = 0; m < ml->len; m++) {
        double *mval = ml->mlist[m];
        for (i = 1; i < ml->slen; i++) {
                vl[m][i] = (normalize(ml->m[m], mval[i]) 
                            - opt.threshold_arg);
        }
    }

    *vote_l = vl;
    *vote_r = NULL;
    return  1;
}
