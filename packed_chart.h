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

#ifndef _PACKED_CHART_H
#define _PACKED_CHART_H       1

#include "lexicon.h"
#include "cyk.h"

/*
 * Each location in the chart is a list of `chart_node's linked with ->next
 *   Each node in the list is headed by a category, contains a list of
 *   back references <L,R> to children linked with ->back fron the head node
 *   Categories in the node_list are unique, different ways of
 *   obtaining the category is listed in the children list.
 */

struct backlink {
        struct chart_node *L; 
        struct chart_node *R;
        struct backlink *next;
};

struct chart_node {
    cg_cat                  *cat;     // category 
    struct chart_node       *next;   // next node with a different category
    struct backlink         *back;  // backlinks to children
    unsigned short      sp_start,  // span start and span length repeated
                        sp_len;   // for convenience during decoding/printing
};                              

struct chart {
    int                 size;
    char                **input;
    struct chart_node   ***node;
};

// data structures for recovered parses from the chart

struct parse_tree {
    struct chart_node   *node;
    struct backlink     *b;
    struct parse_tree   *L;
    struct parse_tree   *R;
    unsigned char       expanded;
};

struct parse_list {
    size_t              parse_num;
    struct parse_tree   *t;
    struct parse_list   *next;
};

struct chart *chart_new(unsigned short size);
void chart_free(struct chart *chart);

struct chart_node *chart_node_add(struct chart *c,
                                   unsigned short i, 
                                   unsigned short j,
                                   cg_cat *cat, 
                                   struct chart_node *bL, 
                                   struct chart_node *bR);

void chart_write(FILE *fp, struct chart *chart);
void write_parses(FILE *fp, struct chart *chart);
void write_parse(FILE *fp, struct parse_tree *t, struct chart *chart);


#endif // _PACKED_CHART_H
