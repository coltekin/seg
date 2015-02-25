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
 * The seglist structure holds an array of segmentations each
 * of which are an array of offset numbers indicating the location
 * of the boundaries. The members of the strucute are
 *   nsegs      Number of segmentations (length of segs array)
 *   nalloc     Internal. keeps the allocated memory
 *   **segs     An array of segmentation arrays
 *              Each member of the array is an array of integers where
 *              the first item is the length of the array. The borders of
 *              the complete string <0,len(s)> is not included in the
 *              segmentation array.
 *              
 */
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "strutils.h"
#include "seglist.h"
#define   ABS(N)    ( (N) >= 0 ? (N) : -(N) )

int seg_check (unsigned short *seg, unsigned short pos)
{
    unsigned short i;
    if (seg == NULL || seg[0] == 0) return 0;
    for (i = 1; i <= seg[0]; i ++) {
        if(seg[i] == pos) return 1;
        if(seg[i] > pos)  return 0;
    }
    return 0;
}

struct seglist *seglist_new()
{
    struct seglist *new;

    new = malloc(sizeof (struct seglist));
    new->nsegs = new->nalloc = 0;
    new->segs = NULL;
    new->score = NULL;
    return new;
}

void
seglist_free(struct seglist *segl)
{
    int i;

    for(i=0; i < segl->nsegs; i++) {
        free(segl->segs[i]);
    }
    free(segl->score);
    free(segl->segs);
    free(segl);
}

void
seglist_add(struct seglist *segl, unsigned short *seg)
{
    int i;
    unsigned short *newseg = NULL;
    
    if (seg != NULL) {
        newseg = malloc((seg[0] + 1) * sizeof(*seg));

        for(i=0; i <= seg[0]; i++) {
            newseg[i] = seg[i];
        }
    }

    segl->nsegs++;

    if(segl->nsegs  >= segl->nalloc ) {
        int alloc_len = BUFSIZ / sizeof (*segl->segs);
        segl->segs = realloc(segl->segs, 
                             (segl->nalloc + alloc_len) * sizeof (*segl->segs));
        assert(segl->segs != NULL);
        segl->score = realloc(segl->score, 
                            (segl->nalloc + alloc_len) * sizeof (*segl->score));
        assert(segl->score != NULL);
        segl->nalloc += alloc_len;
    }

    segl->segs[segl->nsegs - 1] = newseg;
    segl->score[segl->nsegs - 1] = 0.0;
}

void
seglist_add_signed(struct seglist *segl, short *seg)
{
    unsigned short *new;
    int i;

    if (seg != NULL) {
        new = malloc((*seg + 1) * sizeof (*new));
        for (i = 0; i <= seg[0]; i++) {
            new[i] = ABS(seg[i]);
        }
        seglist_add(segl, new);
        free(new);
    } else {
        seglist_add(segl, NULL);
    }
}

inline void
print_intarray(unsigned short *a)
{
    int j;

    if(a == NULL){
        printf("<>\n");
        return;
    }
    printf("<");
    for (j = 0; j <= a[0]; j++) {
        printf("%d,", a[j]);
    }
    printf("\b>\n");
}
