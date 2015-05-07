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

struct lm_options {
    float alpha;       // the paremeter
    struct trie *lex;  // holds the lexical units discovered so far
    size_t *u_count;   // hods the counts of each basic unit (phone or syl.)
    size_t nunits;     // total number of basic units
};

struct seg_handle *lm_init(struct input *in, float alpha, enum seg_unit unit);

struct segmentation *segment_lm(struct seg_handle *h, size_t i);

void lm_cleanup(struct seg_handle *h);

/**
 * lm_segment_range_incremental - segment the given range in the input 
 *                                with the current model and update
 *                                model parameters.
 * @h:                            segmentation handle.
 * @start:                        index of the first utterance to be 
 *                                segmented
 * @end:                          index of the last utterance to be segmented
 * @out:                          an array of segmentation structures that is
 *                                enough to hold the given range.
 *
 *          note: the given range is inclusive. 'start=0, end=0' will
 *          process the utterance with index 0.
 */
void lm_segment_range_incremental(struct seg_handle *h, 
        size_t start, size_t end,
        struct segmentation **out);

/**
 * lm_segment_incremental - segment the given full input with the current 
 *                          model, while updateing the model
 *                          parameters.
 * @h:                      segmentation handle.
 * @out:                    an array of segmentation structures that is
 *                          enough to hold the given range.
 */
void lm_segment_incremental(struct seg_handle *h, struct segmentation **out);

/**
 * lm_segment_range - segment the given range in the input with the
 *                    current model.
 * @h:                segmentation handle.
 * @start:            index of the first utterance to be segmented
 * @end:              index of the last utterance to be segmented
 * @out:              an array of segmentation structures that is
 *                    enough to hold the given range.
 *
 *          note: the given range is inclusive. 'start=0, end=0' will
 *          process the utterance with index 0.
 */
void lm_segment_range(struct seg_handle *h, 
        size_t start, size_t end,
        struct segmentation **out);

/**
 * lm_segment - segment the given full input with the current model.
 * @h:                segmentation handle.
 * @out:              an array of segmentation structures that is
 *                    enough to hold the given range.
 */
void lm_segment(struct seg_handle *h, struct segmentation **out);

/**
 * lm_estimate_range - estimate the model parameters from the given
 *                     range in the input .
 * @h:                 segmentation handle.                    
 * @start:             index of the first utterance to be used for estimation
 * @end:               index of the last utterance to be used for estimation
 *
 *          note: the given range is inclusive. 'start=0, end=0' will
 *          process the utterance with index 0.
 */
void lm_estimate_range(struct seg_handle *h, size_t start, size_t end);

/**
 * lm_estimate - estimate the model parameters from the complete
 *               input.
 * @h:           segmentation handle.                    
 */
void lm_estimate(struct seg_handle *h);

/**
 * lm_write_model - write the model paramters to a file.
 * @h:           segmentation handle.                    
 * @filename:    name of the file
 */
void lm_write_model(struct seg_handle *h, char *filename);


#endif // _SEG_LM_H


