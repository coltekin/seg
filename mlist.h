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

#ifndef _MLIST_H
#define _MLIST_H 1

#include <stdio.h>
#include "phonstats.h"
#include "measures.h"


struct mlist {
    char *s;         //pointer to the (utterance) string 
    int  slen;       //lenght of s, to avoid re-calculation
    size_t   len;    // number of the measures
    size_t   nalloc; // amount of memory used (internal use)
    struct mdata **m;
    double **mlist;
};

struct mlist *mlist_new(int len);
void mlist_free(struct mlist *ml, int free_mdata);
void mlist_add(struct mlist *ml, struct mdata *m);
void mlist_add_old(struct mlist *ml, struct mdata *m, struct phonstats *ps);
void mlist_add2(struct mlist *ml, struct mdata *m, double *val);
void mlist_print(FILE *fp, struct mlist *ml);

#endif // ifndef _MLIST_H
