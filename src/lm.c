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

/*
 * The functions here do "language modeling" style incremental 
 * segmentation. 
 *
 * The probability of a segmentation is simply the 
 * multiplication of the probabilities of the words used in that
 * segmentation.
 *
 * If a word is already in the lexicon, the probability of the 
 * word is its relative frequency times a parameter `alpha'.
 *
 * If the word is unknown, the probability is multiplication of 
 * the probabilities of the phonemes used in the word times 
 * `1 - alpha'.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "input.h"
#include "seg.h"
#include "lm.h"
#include "xalloc.h"

static struct input *inp_dbg;
size_t idx_dbg; // REMOVE ME

static void segment_lm_update(struct unitseq *u, struct segmentation *seg, 
        struct seg_lm_options *o);

static double wordscore(struct unitseq *u, size_t first, size_t last, 
        struct seg_lm_options *o)
{
    double  score;
    size_t i;


    score = trie_relfreq(o->lex, u->seq + first, last - first + 1);

    if (score != 0.0) { // existing word
        score = log(o->alpha) + log(score);
    } else {            // new word
        score = log(1 - o->alpha);
        for (i = first; i <= last; i++) {
            score += log ((double) (o->u_count[u->seq[i]] + 1) /
                          (double) (o->nunits + 1));
        }
    }
    return score;
}

struct seg_handle *segment_lm_init(struct input *in, float alpha,
        enum seg_unit unit)
{
    struct seg_handle *h = malloc(sizeof *h);
    struct seg_lm_options *o = malloc(sizeof *o);
    struct trie *t = trie_init(in->sigma_len + 1);

    h->method = SEG_LM;
    h->in = in;
    h->unit = unit;
    h->options = o;
    o->alpha = alpha;
    o->lex = t;
    o->nunits = (unit == SEG_SYL) ? in->sigma_nsyl - 2 : 
                                    in->sigma_nph - 2;
//    o->u_count = xcalloc(in->sigma_len + 1, sizeof *o->u_count);
    o->u_count = xmalloc((in->sigma_len + 1) * sizeof *o->u_count);
    size_t i;
    for (i = 0; i < in->sigma_len + 1; i++) o->u_count[i] = 1;

    inp_dbg = in; // REMOVE ME

    return h;
}

struct segmentation * 
segment_lm(struct seg_handle *h, size_t idx)
{
    struct utterance *u = h->in->u[idx];
    struct unitseq *seq = (h->unit == SEG_PHON) ?  u->phon : u->syl;
    int len =(h->unit == SEG_PHON) ? u->phon->len : u->syl->len;
    int j, firstch, lastch;
    double  best_score[len];
    size_t  best_start[len];
    struct segmentation *seg = NULL;
    size_t nsegs;
    struct seg_lm_options *opt = h->options;

    idx_dbg = idx; // REMOVE ME

    for (j = 0; j < len; j++) {
        best_score[j] = 0.0;
        best_start[j] = 0;
    }

    // these loops are almost verbatim copies from Brent's (1999) 
    // search algorithm.
    // first: calculate best word ending in each possible end point.
    for (lastch = 0; lastch < len; lastch++){
        best_score[lastch] = wordscore(seq, 0, lastch, opt);
        best_start[lastch] = 0;
        for (firstch = 1; firstch <= lastch; firstch++) {
            double wordsc = wordscore(seq, firstch, lastch, opt);
            if (wordsc + best_score[firstch - 1] > best_score[lastch]) {
                best_score[lastch] = wordsc + best_score[firstch - 1] ;
                best_start[lastch] = firstch;
            }
        }
    }

    // second: insert boundaries starting from the back.
    //
    // first pass: get the number of segments
    firstch = best_start[len - 1];
    nsegs = 0;
    while (firstch > 0) {
        nsegs += 1;
        firstch = best_start[firstch - 1];
    }


    if (nsegs != 0 ) {
        seg = malloc(sizeof *seg);
        seg->len = nsegs;
        seg->bound = malloc(seg->len * sizeof *seg->bound);
        lastch = len - 1;
        firstch = best_start[lastch];
        j = seg->len;
        for (j = seg->len - 1; j >= 0; j--) {
            seg->bound[j] = firstch;
            lastch = firstch - 1;
            firstch = best_start[lastch];
        }
    }

    segment_lm_update(seq, seg, opt);

    return seg;
}

static void segment_lm_update(struct unitseq *u, struct segmentation *seg, 
        struct seg_lm_options *o)
{
    size_t i, j, first, last;

    first = 0;
    for (i = 0; seg != NULL && i < seg->len; i++) {
        last = seg->bound[i];
        trie_insert(o->lex, u->seq + first,  last - first);
        for (j = first; j < last; j++) {
            o->u_count[u->seq[j]] += 1;
        }
        o->nunits += last - first;
        first = last;
    }
    trie_insert(o->lex, u->seq + first, u->len - first);
    for (j = first; j < u->len; j++) {
        o->u_count[u->seq[j]] += 1;
    }
    o->nunits +=  u->len - first;
}

void segment_lm_cleanup(struct seg_handle *h)
{
    struct seg_lm_options *opt = h->options;

    free(opt->u_count);
    trie_free(opt->lex);
    free(opt);
    free(h);
}
