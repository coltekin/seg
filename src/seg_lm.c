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
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "cmdline.h"
#include "lexicon.h"
#include "strutils.h"
#include "input.h"
#include "seg.h"
#include "seg_lm.h"


static double wordscore(segunit_t *seq, int first, int last, 
        struct seg_lm_options *o)
{
    double  score;

    score = trie_relfreq_span(o->lex, seq, first, last - first +1);

    if (score != 0.0) { // existing word
        score = log(o->alpha) + log(score);
    } else {            // new word
        size_t i;
        score = log(1 - o->alpha);
        for (i = first; i <= last; i++) {
            score += log ((double) (o->u_count[seq[i]] + 1) /
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
    h->options = o;
    o->alpha = alpha;
    o->lex = t;
    o->nunits = 0;
    o->u_count = calloc(in->sigma_len, sizeof *o->u_count);

    return h;
}

struct seglist * 
segment_lm(struct seg_handle *h, int idx)
{
    struct utterance *u = h->in->u[idx];
    segunit_t *seq = (h->unit == SEG_PHON) ?  u->phon : u->syl;
    int len =(h->unit == SEG_PHON) ? u->nphon : u->nsyl;
    struct seglist *segl;
    int j, firstch, lastch, nsegs;
    double  bestsc[len];
    int  bestst[len];
    segunit_t seg[len];
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
    nsegs = 0;
    while (firstch > 0) {
        nsegs++;
        firstch = bestst[firstch - 1];
    }


    segl = seglist_new();
    // second pass: add them to segment list
    if (nsegs) {
        seg[0] = nsegs;
        lastch = len;
        firstch = bestst[lastch];
        while (nsegs) {
            seg[nsegs] = firstch;
            lastch = firstch - 1;
            firstch = bestst[lastch];
            nsegs--;
        }
        seglist_add(segl, seg);
    } else {
        segl->nsegs = 1;
        segl->segs = malloc(sizeof *segl->segs);
        segl->segs[0] = NULL;
    }

    segment_lm_update(seq, segl, opt);

    return segl;
}

void segment_lm_update(segunit_t *seq, struct seglist *segl, 
        struct seg_lm_options *o)
{
    size_t i, j, first, last;
    segunit_t *seg = segl->segs[0];

    first = 0;
    for (i = 0; i < seg[0]; i++) {
        last =  seg[i + 1];
        trie_insert_span(o->lex, seq, first, last - first);
        for (j = first; j < last; j++) {
            o->u_count[seq[j]] += 1;
        }
        o->nunits += last - first;
        first = last;
    }
    trie_insert_span(o->lex, seq, first, 0);
    for (j = first; ; j++) {
        if (seq[j] == 0) break;
        o->u_count[seq[j]] += 1;
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
