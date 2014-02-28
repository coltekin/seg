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

#ifndef _CTXLEX_H
#define _CTXLEX_H 1
#include "cmdline.h"
#include "options.h"
#include <glib.h>
#include "prob_dist.h"


struct lexdata {
    char *s;
    size_t freq,
           nctx;
    GHashTable  *ctx;
};

struct lexstats {
    size_t max_wlen;
    int    nalloc;
    struct prob_dist *wdist;
    struct prob_dist *ctxdist;
    struct prob_dist **wldist;
    struct prob_dist **ctxldist;
};

struct ctxlex {
    size_t wntok,
           wntyp,
           nctx;
    struct lexdata **lex;
    int    n;
    size_t nalloc;
    struct lexstats *stats;
    GHashTable *lexhash;
    GHashTable *ctxhash;
};

struct ctxlex *ctxlex_new();
struct lexdata *ctxlex_add(struct ctxlex *lex, char *s, char *l, char *r);
size_t ctxlex_freq(struct ctxlex *lex, char *s);
size_t ctxlex_nctx(struct ctxlex *lex, char *s);
double ctxlex_freq_z(struct ctxlex *lex, char *s);
double ctxlex_nctx_z(struct ctxlex *lex, char *s);
void ctxlex_free(struct ctxlex *cl);

#endif // _CTXLEX_H

