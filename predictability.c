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
 * This file includes routines to calculate various predictability measures.
 *
 * All functions calculate respective measures for a given string xy, 
 * where x, and y are substrings. 
 *
 * The functions accept a complete string (s), the position offset where 
 * x ends and y begins (pos), length of x (x_len) and length of y (y_len).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "predictability.h"
#include "pred.h"

struct pred_minfo p_info[]={
{.sname = "jp", 
 .lname = "Joint Probability",
 .mmask = 0x0001},
{.sname = "tp",
 .lname = "Transitional Probability",
 .mmask = 0x0002},
{.sname = "mi",
 .lname = "Pointwise mutual information",
 .mmask = 0x0004},
{.sname = "sv",
 .lname = "Successor Variety",
 .mmask = 0x0010},
{.sname = "h",
 .lname = "Conditional Entropy",
 .mmask = 0x0008},
{.sname = "rtp",
 .lname = "Reverse Transitional Probability",
 .mmask = 0x0200},
{.sname = "rsv",
 .lname = "Predecessor variety",
 .mmask = 0x1000},
{.sname = "rh",
 .lname = "Reverse Conditional Entropy",
 .mmask = 0x0400},
};

double *
pred_list(struct phonstats *ps, 
          char *s, 
          enum pred_measure m, 
          int x_len, int y_len)
{
    int len = strlen(s);
    int slen = len + 3;
    double *mlist = calloc(1 + len, sizeof (*mlist));
    char xstr[slen];
    char ystr[slen];
    char *x = NULL, *y = NULL;
    int pos;
    unsigned short need_y = (m == PM_JP || m == PM_TP || m == PM_MI ||
                             m == PM_HR || m == PM_SVR|| m == PM_TPR);
    unsigned short need_x = (m == PM_JP || m == PM_TP || m == PM_MI ||
                             m == PM_H  || m == PM_SV || m == PM_TPR);

    assert(m < PM_MAX);
    if (need_x) {
        add_bow_eow(xstr, s);
    }

    if (need_y) {
        add_bow_eow(ystr, s);
    }

    for (pos = 0; pos <= len; pos++) {
        if(need_x) {
            x = (pos - x_len < 0) ? xstr
                                  : xstr + pos - x_len + 1;
            char_swap(xstr, slen, pos + 1);
        }

        if (need_y) {
            y = ystr + pos + 1;
            if (1 + pos + y_len < slen) {
                char_swap(ystr, slen, pos + y_len + 1);
            }
        }

        mlist[pos] = pred_calc(ps, m, x, y, x_len, y_len);
        
        if (need_x) {
            char_unswap(xstr, slen, pos + 1);
        }
        if (need_y) {
            if (1 + pos + y_len < slen) {
                char_unswap(ystr, slen, pos + y_len + 1);
            }
        }
    }

    return mlist;
}


struct mlist_list *
pred_mlist_list_new(int llen)
{
    struct mlist_list *ml = malloc(sizeof(*ml));
    if(llen) {
        ml->llen = llen;
        ml->x = malloc(llen * sizeof(*ml->x));
        ml->y = malloc(llen * sizeof(*ml->y));
        ml->m = malloc(llen * sizeof(*ml->m));
        ml->mlist = malloc(llen * sizeof(*ml->mlist));
    }
    return ml;
}

void
pred_mlist_list_free(struct mlist_list *ml)
{
    int i;
    for (i = 0; i < ml->llen; i++) {
        free(ml->mlist[i]);
    }
    free(ml->x);
    free(ml->y);
    free(ml->m);
    free(ml);
    ml = NULL;
}

struct mlist_list *
pred_mlist_list(struct phonstats *ps, 
                char *s,
                unsigned mmask,
                int xmin, int xmax,
                int ymin, int ymax)
{
    int llen =  (xmax - xmin + 1) * (ymax - ymin + 1)
               * __builtin_popcount(mmask);
    int x, y, i = 0;
    unsigned m = 0;
    struct mlist_list *ml = pred_mlist_list_new(llen);

    for(x = xmin; x <= xmax; x++) {
        for(y = ymin; y <= ymax; y++) {
            for(m = 0; m < PM_MAX; m++) { 
                if(p_info[m].mmask & mmask) {
                    ml->x[i] = x;
                    ml->y[i] = y;
                    ml->m[i] = m;
                    // if measure is a reverse measure swap the context size
                    if(p_info[ml->m[i]].mmask & PM_REVMASK) {
                        ml->mlist[i] = pred_list(ps, s, m, y, x);
                    } else {
                        ml->mlist[i] = pred_list(ps, s, m, x, y);
                    }
                    i++;
                }
            }
        }
    }

    return ml;
}

