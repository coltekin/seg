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
#include "mdata.h"
#include "measures.h"


struct mdata *
mdata_new()
{
    struct mdata *md = malloc (sizeof (*md));
    md->info = NULL;
    md->s = NULL;
    md->len_l = md->len_r = 0;
    md->w_l = md->w_r = 1;
    md->L = NULL;
    md->cL = NULL;
    md->c = NULL;
    md->ps = NULL;
    return md;
}

struct mdata *
mdata_new_full(enum m_id mid, char *s, short len_l, short len_r)
{
    struct mdata *md = mdata_new();
    md->info = &m_info[mid];
    md->s = s;
    md->len_l = len_l;
    md->len_r = len_r;
    return md;
}

void 
mdata_free(struct mdata *md)
{
    free(md);
}

struct mdlist *
mdlist_new()
{
    struct mdlist *mdl = malloc (sizeof (*mdl));
    mdl->md = NULL;
    mdl->n = mdl->nalloc = 0;
    
    return mdl;
}

void
mdlist_add (struct mdlist *mdl, struct mdata *md)
{
    if(mdl->n  >= mdl->nalloc ) {
        mdl->nalloc += BUFSIZ;
        mdl->md = malloc (mdl->nalloc * sizeof (*mdl->md));
    }
    
    mdl->md[mdl->n] = md;
    mdl->n++;
}

void
mdlist_free(struct mdlist *mdl)
{
    int i;

    for (i = 0; i < mdl->n; i++) {
        mdata_free(mdl->md[i]);
    }
    free(mdl->md);
    free(mdl);
}
