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
 * word its relative frequency times a parameter `alpha'.
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
#include "io.h"
#include "phonstats.h"
#include "seg.h"
#include "seg_lm.h"

static cg_lexicon *L;
static struct phonstats *ps;

static double 
wordscore(char *u, int firstch, int lastch, double alpha)
{
    char    *w = str_span(u, firstch, lastch-firstch+1);
    double  score;

    score = cg_lexicon_get_rfreq_pf(L, w);

    if (score != 0.0) { // existing word
        score = log(alpha) + log(score);
    } else {            // new word
        char *tmp = w;
        score = log(1 - alpha);
        while (*tmp) {
            score += log(phonstats_rfreq_p(ps, *tmp));
            tmp++;
        }
    }
    free(w);
    return score;
}

void 
segment_lm_init(struct input *in)
{
    // TODO: input lexicon?
    L = cg_lexicon_new();
    ps = phonstats_new(1, "IE&AaOU6ie9Quo73R#%*()pbmtdnkgNfvTDszSZhcGlrL~MywW");
}

struct seglist * 
segment_lm(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    struct seglist *segl;
    int j, firstch, lastch, nsegs;
    int  len = strlen(u) - 1;
    double  bestsc[len];
    int  bestst[len];
    unsigned short    seg[len];
    float alpha = opt.alpha_arg;

    for (j = 0; j < len; j++) {
        bestsc[j] = 0.0;
        bestst[j] = 0;
    }

    // these loops are almost verbatim copies from Brent's (1999) 
    // search algorithm.
    // first: calculate best word ending in each possible end point.
    for (lastch = 0; lastch <= len; lastch++){
        bestsc[lastch] = wordscore(u, 0, lastch, alpha);
        bestst[lastch] = 0;
        for (firstch = 1; firstch <= lastch; firstch++) {
            double wordsc = wordscore(u, firstch, lastch, alpha);
            if (wordsc + bestsc[firstch - 1] > bestsc[lastch]) {
                bestsc[lastch] = wordsc + bestsc[firstch - 1] ;
                bestst[lastch] = firstch;
            }
        }
    }

    // second: insert boundaries starting from the back.
    //
    // first pass: get the number of segmetns
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
//            printf("%.*s=%f ", 
//                    lastch - firstch + 1, u + firstch, 
//                    wordscore(u, firstch, lastch, alpha));
            lastch = firstch - 1;
            firstch = bestst[lastch];
            nsegs--;
        }
//        printf("%.*s=%f\n", 
//                lastch - firstch + 1, u + firstch, 
//                wordscore(u, firstch, lastch, alpha));
        seglist_add(segl, seg);
//printf("DDD: %s ", u);
//print_intarray(seg);
    } else {
        segl->nsegs = 1;
        segl->segs = malloc(sizeof *segl->segs);
        segl->segs[0] = NULL;
//printf        printf("%s=%f\n", u, wordscore(u, 0, len, alpha));
    }

    segment_lm_update(u, segl);

    return segl;
}

void 
segment_lm_update(char *s, struct seglist *segl)
{
    char **segstr;
    char **seg;
    assert(segl->nsegs == 1);

    segstr = seg_to_strlist(s, segl->segs[0]);
    seg = segstr;
    while (*seg) {
        cg_lexicon_add(L, *seg, "C", NULL);
        phonstats_update(ps, *seg);
        ++seg;
    }
    free_strlist(segstr);
}

void 
segment_lm_cleanup()
{
    return;
}
