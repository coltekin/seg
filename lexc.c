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

#include <assert.h>
#include <math.h>
#include <string.h>
#include "seglist.h"
#include "segparse.h"
#include "lexc.h"


static double
wscore(cg_lexicon *L, char *w, char *pw, char *nw)
{
    assert (pw == NULL && nw == NULL);

    size_t freq = cg_lexicon_get_freq_pf(L, w);
    if (freq > 0) {
        return log((double) (freq + 1));
    } else {
        return 0;
    }
}

static double
uscore(unsigned short *seg, char *u)
{
    int nsegs = seg[0] + 1;
    return 1.0 / (double) nsegs;
}

void
lexc_segl_score(struct seglist *segl, 
              cg_lexicon *L, 
              struct phonstats *ps, 
              struct phonstats *lps, 
              char *u)
{
    int i, j;

    for (i = 0; i < segl->nsegs; i++) {
        double score=0.0;
        char **words = seg_to_strlist(u, segl->segs[i]);
        int nsegs = segl->segs[i][0] + 1;
        for (j = 0; j < nsegs; j++) {
            score += wscore(L, words[j], NULL, NULL);
        }
        segl->score[i] = (uscore(segl->segs[i], u) * score) / (double) nsegs;
        free_strlist(words);
    }
}

unsigned short *
lexc_best_seg(cg_lexicon *L, 
             struct phonstats *ps, 
             struct phonstats *lps, 
             char *u)
{
    struct chart *c = seg_parse(L, u, seg_combine);
    struct seglist *segl;
    int best_seg = 0;
    double max_score = 0.0;
    unsigned short *seg;
    int i;

    if (!opt.lexicon_partial_given) { // default is partial segmentation
        opt.lexicon_partial_arg = lexicon_partial_arg_all;
    }

    switch (opt.lexicon_partial_arg) {
        case lexicon_partial_arg_all:
            segl = get_segs_partial_opt(c, SPOPT_ALL);
        break;
        case lexicon_partial_arg_one:
            segl = get_segs_partial_opt(c, SPOPT_ONE);
        break;
        case lexicon_partial_arg_begin:
            segl = get_segs_partial_opt(c, SPOPT_BEGIN);
        break;
        case lexicon_partial_arg_end:
            segl = get_segs_partial_opt(c, SPOPT_END);
        break;
        case lexicon_partial_arg_beginend:
            segl = get_segs_partial_opt(c, SPOPT_BEGINEND);
        break;
        case lexicon_partial_arg_none:
        default:
            segl = get_segs_full(c);
        break;
    }

    lexc_segl_score(segl, L, ps, lps, u);

    for (i = 0; i < segl->nsegs; i++) {
        if (segl->score[i] > max_score) {
            max_score = segl->score[i];
            best_seg = i;
        }
    }

    int seglen = (1 + segl->segs[best_seg][0]) * sizeof (*seg);
    seg = malloc(seglen);
    memcpy(seg, segl->segs[best_seg], seglen);

    return seg;
}
