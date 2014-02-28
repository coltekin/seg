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

#ifndef _LEXC_H
#define _LEXC_H 1
#include "options.h"
#include "lexicon.h"
#include "phonstats.h"


void lexc_segl_score(struct seglist *segl,
                     cg_lexicon *L,
                     struct phonstats *ps,
                     struct phonstats *lps,
                      char *u);

unsigned short *lexc_best_seg(cg_lexicon *L, 
                              struct phonstats *ps, 
                              struct phonstats *lps, 
                              char *u);


#endif // _LEXC_H

