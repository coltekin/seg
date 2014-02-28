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

#ifndef _CYK_H
#define _CYK_H       1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexicon.h"

typedef struct cyk_chart_node {
    cg_cat                  *cat;           // category 
    struct cyk_chart_node   *backL, *backR; //backlinks for children
    struct cyk_chart_node   *next;
    int                     i, j;   // span j: span start, i: span len
} cyk_chart_node;

typedef struct cyk_chart {
    int            size;
    char           **input;
    cyk_chart_node ***node;
} cyk_chart;

cg_cat *cg_combine(cg_cat *catL, cg_cat *catR);
cyk_chart_node *cyk_node_new(cg_cat *cat, 
                              cyk_chart_node *bL, cyk_chart_node *bR, 
                              cyk_chart_node *next, 
                              int i, int j);
cyk_chart *cyk_chart_new(int size);
cyk_chart *cyk_parse(cg_lexicon *l, char **input, size_t inplen);
void cyk_chart_write(FILE *fp, cyk_chart *chart);
void cyk_write_parses(FILE *fp, cyk_chart *chart);
void cyk_chart_free(cyk_chart *chart);



#endif /* _CYK_H */
