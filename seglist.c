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
#include "options.h"
#include "strutils.h"
#include "seglist.h"
#define   ABS(N)    ( (N) >= 0 ? (N) : -(N) )

/* seg_check() : check if pos is in `seg', i.e., if we have a 
 * boundary at postion pos.
 */
int
seg_check (unsigned short *seg, unsigned short pos)
{
    unsigned short i;
    if (seg == NULL || seg[0] == 0) return 0;
//printf("\n\t\t[%hu/%hu]: ", seg[0], pos);
//for(i = 1; i <= seg[0]; i++) {
//    printf("%hu, ", seg[i]);
//}
//printf("\n");

    for (i = 1; i <= seg[0]; i ++) {
        if(seg[i] == pos) return 1;
        if(seg[i] > pos)  return 0;
//        if(seg[i] == pos) {printf("\t\t%hu == %hu: yay!\n",seg[i], pos);return 1;}
//        if(seg[i] > pos) {printf("\t\t%hu == %hu: ney!\n",seg[i], pos);return 0;}
    }
    return 0;
}

struct seglist *
seglist_new()
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
seglist_shuffle(struct seglist *segl)
{
    int i;
    if (segl->nsegs <= 1) {
        return;
    }

    if (opt.shuffle_arg == -1) {
        srand(time(NULL));
    } else {
        srand(opt.shuffle_arg);
    }

    for (i = 0; i < segl->nsegs - 1; i++) {
        size_t j = i + rand() / (RAND_MAX / (segl->nsegs - i) + 1);
        unsigned short *tseg;
        double tsc;
        tseg = segl->segs[j];
        segl->segs[j] = segl->segs[i];
        segl->segs[i] = tseg;
        tsc = segl->score[j];
        segl->score[j] = segl->score[i];
        segl->score[i] = tsc;
    }
 
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

char **
seg_to_strlist(char *s, unsigned short *seg)
{
    char **ret;
    int  nsegs;
    int  slen = strlen(s);
    int firstch, lastch, i;

    if(seg == NULL || seg[0] == 0) {
        ret = malloc (2 * sizeof *ret);
        ret[0] = strdup(s);
        ret[1] = NULL;
        return ret;       
    }

    nsegs = seg[0];
    
    ret = malloc((nsegs + 2) * sizeof *ret);
    firstch = 0;
//printf("str_to_seg: %s ", s);
//print_intarray(seg);
    for(i = 0; i < nsegs; i++) {
        lastch = seg[i + 1];
        ret[i] = str_span(s, firstch, lastch - firstch);
        firstch = lastch;
//printf("\tstr_to_seg: %s\n", ret[i]);
    }
    lastch = slen;
    ret[nsegs] = str_span(s, firstch, lastch - firstch);
//printf("\tstr_to_seg: %s\n", ret[nsegs]);
    ret[nsegs + 1] = NULL;
    return ret;
}

void 
free_strlist(char **s)
{
    char **tmp = s;
    while(*tmp) {
        free(*tmp);
        tmp++;
    }
    free(s);
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


void
seglist_write(FILE *fp, struct seglist *segs)
{   
    int     i, j;

    printf(" [%d]: ", segs->nsegs);
    for (i = 0; i < segs->nsegs; i++) {
        printf("<");
        for (j = 0; j <= segs->segs[i][0]; j++) {
            printf("%d,", segs->segs[i][j]);
        }
        printf("\b> ");
    }
    printf("\n");
}

#define SEG_SEP " "
#define SEGS_SEP " | "

void
seglist_print_segs(FILE *fp, struct seglist *segl, char *s)
{
    int j;

    for (j = 0; j < segl->nsegs; j++) {
        char **segstr = seg_to_strlist(s, segl->segs[j]);
        char **tmp = segstr;
        while (*tmp) {
            fprintf(fp, "%s", *tmp);
            tmp++;
            if (*tmp) fprintf(fp, "%s", SEG_SEP);
        }
        if (j < segl->nsegs - 1) {
            fprintf(fp, "%s", SEGS_SEP);
        }
        free_strlist(segstr);
    }
    fprintf(fp, "\n");
}
