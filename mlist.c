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

/* Generic functions to handle `masures' and a list of `measures' for 
 * each boundary position.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mlist.h"
#include "mdata.h"


struct mlist *
mlist_new(int mcount)
{
    struct mlist *ml = malloc(sizeof (struct mlist));

    ml->s = NULL;
    ml->len = 0;
    ml->slen = 0;
    ml->m = (mcount == 0) ? NULL : calloc(mcount, sizeof (*ml->m));
    ml->mlist = (mcount == 0) ? NULL : calloc(mcount, sizeof (*ml->mlist));
    ml->nalloc = (mcount == 0) ? 0 : mcount;
    return ml;
}

void 
mlist_free(struct mlist *ml, int free_data)
{
    int i;

    for (i = 0; i < ml->len; i++) {
        if (free_data) {
            free(ml->m[i]);
            ml->m[i] = NULL;
        }
        free(ml->mlist[i]);
    }
    free(ml->mlist);
    free(ml->m);
    free(ml);
}

/* mlist_add2() : add the specified measure values to given mlist
 */

#define ALLOC_INCR (BUFSIZ / sizeof (*ml->mlist))
void mlist_add2(struct mlist *ml, struct mdata *m, double *val)
{

    if (ml->nalloc == ml->len) {
        ml->nalloc += ALLOC_INCR;
        ml->m = realloc(ml->m, ml->nalloc * sizeof (*ml->m));
        ml->mlist = realloc(ml->m, ml->nalloc * sizeof (*ml->mlist));
    }

    ml->m[ml->len] = m;
    ml->mlist[ml->len] = val;
    ++ml->len;
}

/* mlist_add() : add the specified measure to given mlist, calculating 
 *               it from the supplied function in mdata
 */

void 
mlist_add(struct mlist *ml, struct mdata *m)
{
    assert (m->info->calc_list != NULL);
    mlist_add2(ml, m,  m->info->calc_list(m->ps, m));
}

void 
mlist_add_old(struct mlist *ml, struct mdata *m, struct phonstats *ps)
{
    assert (m->info->calc_list != NULL);
    mlist_add2(ml, m,  m->info->calc_list(ps, m));
}

#define PREC 2
void mlist_print(FILE *fp, struct mlist *ml)
{
    int i, j;
    int slen = strlen(ml->s);

    fprintf(fp, "%s:\n", ml->s);
    for (i = 0; i < ml->len; i++) {
        fprintf(fp, "%s[%d/%d]: ", ml->m[i]->info->sname, 
                                   ml->m[i]->len_l, ml->m[i]->len_r);
        fprintf(fp, "< %.*f", PREC, ml->mlist[i][0]);
        for (j = 1; j < slen; j++) {
            printf (" %c %.*f", ml->s[j-1], PREC, ml->mlist[i][j]);
        }
        fprintf(fp, " %.*f >\n", PREC, ml->mlist[i][slen]);
    }
}
