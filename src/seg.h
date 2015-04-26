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

#ifndef _SEG_H
#define _SEG_H 1

#include <stdint.h>

/**
 * segunit_t is the type used for indexes of (basic) units in the lexicon.
 * It is an unsigned integer type that is large enough to represent
 * all units used (e.g., all phonemes or syllables). Using the
 * smallest sufficient integer type unit may save lots of memory
 * (e.g., if we are only dealing with phonemes or letters).
 */
typedef uint16_t segunit_t;
#define SEGUNIT_T_MAX UINT16_MAX

/**
 * segfeature_t is a bit mask representing non-segmental features of a
 * phoneme or syllable. The same mask is used
 * for both the types (e.g., having some phonetic features) and tokens
 * (e.g., having stress or not).
 * The relevant fields are defined with SEGFEAT_* below.
 */
typedef uint8_t  segfeature_t;

#define SEGFEAT_STRESS1 1        // primary stress
#define SEGFEAT_STRESS2 (1 << 1) // secondary stress
#define SEGFEAT_ISSYL   (1 << 2) // entry is a syllable, not phoneme

/**
 * segmetation_t is used in specifying boundary offsets for
 * segmentations (in struct segmentation below). Depending on the
 * size of the corpus it may also save some meomry to use the smallest
 * sufficient unit.
 */
typedef uint8_t segmentation_t;
#define SEGMENTATION_T_MAX UINT8_MAX

/**
 * struct segmentation - the structure holding a sequence of boundaries
 * @len:                 number of boundaries.
 * @bound:               an array of boundaries.
 *
 *
 * This can only be interpretted together with a list of basic units,
 * e.g., phonemes.  @bound contains  an ordered list of @len
 * boundaries. The the beginning and the end of the complete sequence
 * is not listed.  Locations are with respect to the units in
 * corresponding utterance, and should be ordered. For example,
 * assuming each letter is a phoneme, the segmentation 'a def gh'
 * should only be represented as {1, 4}. A p
 *
 * A NULL pointer, as well as a structure with @len == 0, is
 * interpreted as having no internal boundaries.
 */

struct segmentation { 
    int len;
    segmentation_t *bound;
};

enum seg_method {
    SEG_LM = 0
};

enum seg_unit {
    SEG_PHON,
    SEG_SYL
};

struct seg_handle {
    enum seg_method method; // method
    enum seg_unit unit;     // basic unit to use for segmentation
    struct input *in;       // the input structure
    void *options;          // method specific options

/**
 * the following function pointers define the interface of a
 * segmentation model. Besides these, a model provides a
 * <model>_init() function where the information below is filled in.
 * See lm.h for function descriptions.
 **/
    void (*segment)(struct seg_handle *h, struct segmentation **out);
    void (*segment_range)(struct seg_handle *h, size_t start, size_t end, 
                         struct segmentation **out);
    void (*segment_incremental)(struct seg_handle *h, 
                                struct segmentation **out);
    void (*segment_range_incremental)(struct seg_handle *h, 
                                     size_t start, size_t end, 
                                     struct segmentation **out);
    void (*estimate)(struct seg_handle *h);
    void (*estimate_range)(struct seg_handle *h, size_t start, size_t end);
    void (*cleanup)(struct seg_handle *h);
};

#endif // _SEG_H

