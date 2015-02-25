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

static void segment_lm_update(struct unitseq *u, struct segmentation *seg, 
        struct seg_lm_options *o);

static double wordscore(struct unitseq *u, int first, int last, 
        struct seg_lm_options *o)
{
    double  score;

    score = trie_relfreq(o->lex, u->seq + first, last - first);

    if (score != 0.0) { // existing word
        score = log(o->alpha) + log(score);
    } else {            // new word
        size_t i;
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
    struct trie *t = (unit == SEG_PHON) ? trie_init(in->sigma_nph) :
        trie_init(in->sigma_nsyl);

    h->method = SEG_LM;
    h->in = in;
    h->unit = unit;
    h->options = o;
    o->alpha = alpha;
    o->lex = t;
    o->nunits = 0;
    o->u_count = calloc(in->sigma_len, sizeof *o->u_count);

    return h;
}

struct segmentation * 
segment_lm(struct seg_handle *h, size_t idx)
{
    struct utterance *u = h->in->u[idx];
    struct unitseq *seq = (h->unit == SEG_PHON) ?  u->phon : u->syl;
    int len =(h->unit == SEG_PHON) ? u->phon->len : u->syl->len;
    int j, firstch, lastch;
    double  bestsc[len];
    int  bestst[len];
    struct segmentation *seg = malloc(sizeof *seg);
    struct seg_lm_options *opt = h->options;

    for (j = 0; j < len; j++) {
        bestsc[j] = 0.0;
        bestst[j] = 0;
    }

    // these loops are almost verbatim copies from Brent's (1999) 
    // search algorithm.
    // first: calculate best word ending in each possible end point.
    for (lastch = 0; lastch <= len; lastch++){
        bestsc[lastch] = wordscore(seq, 0, lastch, opt);
        bestst[lastch] = 0;
        for (firstch = 1; firstch <= lastch; firstch++) {
            double wordsc = wordscore(seq, firstch, lastch, opt);
            if (wordsc + bestsc[firstch - 1] > bestsc[lastch]) {
                bestsc[lastch] = wordsc + bestsc[firstch - 1] ;
                bestst[lastch] = firstch;
            }
        }
    }

    // second: insert boundaries starting from the back.
    //
    // first pass: get the number of segments
    firstch = bestst[len];
    seg->len = 0;
    while (firstch > 0) {
        seg->len += 1;
        firstch = bestst[firstch - 1];
    }


    if (seg->len) {
        seg->bound = malloc(sizeof *seg->bound);
        lastch = len;
        firstch = bestst[lastch];
        j = seg->len;
        for (j = seg->len; j > 0; j--) {
            seg->bound[j] = firstch;
            lastch = firstch - 1;
            firstch = bestst[lastch];
        }
    } else {
        free(seg);
        seg = NULL;
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
    for (j = first; ; j++) {
        if (u->seq[j] == 0) break;
        o->u_count[u->seq[j]] += 1;
    }
}

void segment_lm_cleanup(struct seg_handle *h)
{
    struct seg_lm_options *opt = h->options;
    free(opt->u_count);
    trie_free(opt->lex);
    free(opt);
    free(h);
}
