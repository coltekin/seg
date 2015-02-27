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

#ifndef _SCORE_H
#define _SCORE_H 1

#include <stdint.h>
#include "input.h"
#include "seg.h"

struct seg_counts {
    unsigned  btp, bfp, bfn,
              wtp, wfp, wfn,
              ltp, lfp, lfn;
};

struct seg_score {
    double bp, br,   // boundary precision and recall
           wp, wr,   // word p & r
           lp, lr,   // lexicon p & r
           eo, eu;   // over- and under-segmentation (error) scores
    struct seg_counts c;
};


#define f_score(prec, recall) ((2.0*prec*recall)/(prec+recall))

#define SCORE_OPT_HEADER 1
#define SCORE_OPT_LATEX  2
#define SCORE_EDGES      4

void print_prf(struct input *in, struct segmentation **out, 
        size_t range_start, size_t range_end, uint8_t options);

#endif // _SCORE_H
