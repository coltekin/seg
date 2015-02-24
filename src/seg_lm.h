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

#ifndef _SEG_LM_H
#define _SEG_LM_H 1
#include "seg.h"
#include "trie.h"

struct seg_lm_options {
    float alpha;       // the paremeter
    struct trie *lex;  // holds the lexical units discovered so far
    size_t *u_count;   // hods the counts of each basic unit (phone or syl.)
    size_t nunits;     // total number of basic units
};

struct seg_handle *segment_lm_init(struct input *in, float alpha, 
        enum seg_unit unit);

struct seglist *segment_lm(struct seg_handle *h, int i);

void segment_lm_update(segunit_t *seq, struct seglist *segl, 
        struct seg_lm_options *o);

void segment_lm_cleanup(struct seg_handle *h);



#endif // _SEG_LM_H


