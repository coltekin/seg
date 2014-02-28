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

#ifndef _LEX_H
#define _LEX_H 1

#include "measures.h"
#include "phonstats.h"
#include "mdata.h"

void print_words_before(cg_lexicon *L, struct chart *c, short pos);
void print_words_after(cg_lexicon *L, struct chart *c, short pos);
int words_before(cg_lexicon *L, struct chart *c, short pos, short freq);
int words_after(cg_lexicon *L, struct chart *c, short pos, short freq);

double calc_lex_single(struct phonstats *ps, struct mdata *m, int pos);
double *calc_lex_list(struct phonstats *ps, struct mdata *m);

int lex_init(struct mdlist *mdl, struct cg_lexicon *L, struct phonstats *ps, struct ctxlex *cL);

#endif // _LEX_H
