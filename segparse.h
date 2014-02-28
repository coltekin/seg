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

#ifndef _SEGPARSE_H
#define _SEGPARSE_H       1


#include "lexicon.h"
#include "seglist.h"
#include "cclib_debug.h"
#include "cyk_packed.h"
#include "packed_chart.h"

enum segparse_opt {
    SPOPT_ONE,
    SPOPT_END,
    SPOPT_BEGIN,
    SPOPT_BEGINEND,
    SPOPT_ALL,
};

void seglist_free(struct seglist *segl);

typedef cg_cat * (*combine_funct_t)(cg_cat *, cg_cat*);

cg_cat * seg_combine(cg_cat *L, cg_cat *R);
struct chart * seg_parse(cg_lexicon *l, char *input, combine_funct_t combine);
void write_segs(FILE *fp, struct chart *c);

struct seglist *get_segs_partial_opt(struct chart *chart, enum segparse_opt o);
struct seglist *get_segs_partial(struct chart *chart);
struct seglist *get_segs_full(struct chart *chart);



#endif // _SEGPARSE_H 
