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

#ifndef _NGRAMS_H
#define _NGRAMS_H 1

#include "seg.h"
#include "input.h"
#include "trie.h"

struct ngrams {
    struct trie *trie;
    size_t maxng;
    size_t *ngcount;
    segunit_t  bos; // beginning of sequence marker
    segunit_t  eos; // end of sequence marker
};

/* ngrams_init() - initialize a ngrams structure and return it
 * @sigma_size    size of the alphabet that forms the sequences
 * @maxng         maximum ngram size the statistics are collected for
 *
 */
struct ngrams *ngrams_init(size_t sigma_size, size_t maxng);

/* ngrams_update() - update the ngram statistics from the give sequence
 * @ng               the target ngram structure
 * @seq              sequence to find ngrams
 *
 * This function inserts all subsequences up to length @ng->maxng into 
 * @ng->trie. As well as the symbols in the sequence, two special symbols 
 * are inserted for the beginning and end of sequnces (e.g. utterances).
 *
 */
void ngramss_update(struct ngrams *ng, segunit_t *seq);

/* ngrams_freq() - return the frequency of a given ngram
 */
size_t ngrams_freq(struct phonestats *ng, segunit_t *seq);

/* ngrams_freq_span() - return the frequency of the ngram in @seq
 *                      defined by @first @length. 
 * @ng                  the target ngram structure
 * @seq                 the complte squence where ngram is located
 * @first               the start position of the ngram in the @seq
 * @first               length of the ngram.
 */
size_t ngrams_freq_span(struct phonestats *ng, segunit_t *seq,
        size_t first, size_t length);

/* ngrams_freq_begin() - return the number of occurences of the ngram 
 *                       represented by first @len members of the seq
 *                       at the beginnings of the sequences inserted
 *                       into @ng.
 *
 */
size_t ngrams_freq_begin(struct phonestats *ng, segunit_t *seq, size_t len);

/* ngrams_freq_end() - return the number of occurences of the ngram 
 *                     represented by last @len members of the seq
 *                     at the ends of the sequences inserted
 *                     into @ng.
 *
 */
size_t ngrams_freq_end(struct phonestats *ng, segunit_t *seq, size_t len);

/* ngrams_rfreq() - return the relative frequency of a given ngram
 *                  with add-one smoothing. 
 *
 * It is an error if the length of the sequence is larger than the
 * @ng->maxng. If there is not data for a particular ngram size 0.0 is
 * returned. Otherwise return value is the relative frequency of the
 * given sequence with add-one smoothing.
 */

double ngrams_rfreq(struct phonestats *ng, segunit_t *seq);

/* ngrams_rfreq() - return the relative frequency of a span of the
 *                  given @seq. The arguments are similar to
 *                  @ngrams_freq_span() and the return value is as
 *                  explained for @ngrams_rfreq().
 */
double ngrams_rfreq_span(struct phonestats *ng, segunit_t *seq,
        size_t first, size_t length);

/* ngrams_rfreq_begin() - return the relative frequency of the ngram 
 *                        represented by first @len members of the seq
 *                        as it occurs at the beginnings of the sequences 
 *                        inserted into @ng.
 *
 */
size_t ngrams_freq_begin(struct phonestats *ng, segunit_t *seq, size_t len);

/* ngrams_rfreq_end() - return the relative frequency of the ngram 
 *                     represented by last @len members of the seq
 *                     as it occurs at the ends of the sequences 
 *                     inserted into @ng.
 *
 */
size_t ngrams_freq_end(struct phonestats *ng, segunit_t *seq, size_t len);

#endif // _NGRAMS_H

