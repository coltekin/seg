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

/* lexicon_segment.c
 *
 * This file segments the given input using the lexicon only. 
 * A lexicon has to be provided, which will not be modified: 
 * there is no learning here.
 * 
 * the function lexicon_segment() can be called from other seg_ methods.
 *
 */
#include <assert.h>
#include "segparse.h"
#include "seg_lexicon.h"
#include "seg.h"
#include "lexc.h"
#include "mdata.h"

static cg_lexicon *L;

void 
segment_lexicon_init(struct input *in)
{

    assert(opt.inlex_given);

    L = cg_lexicon_load(opt.inlex_arg);

    if (opt.score_arg == score_arg_best) {
        int i, j;
        for (i = 0; i < in->size; i++) {
            if (in->u[i].seg == NULL || in->u[i].seg[0] == 0) {
                    cg_lexicon_add(L, in->u[i].s, "x", NULL);
            } else {
                char **words = seg_to_strlist(in->u[i].s, in->u[i].seg);
                for (j = 0; j < in->u[i].seg[0]; j++) {
                    cg_lexicon_add(L, words[j], "x", NULL);
                }
                free_strlist(words);
            }
        }
    }
printf ("!!! %d\n", opt.lexicon_partial_given);
}

struct seglist *
lexicon_segment(struct cg_lexicon *l, char *u)
{
    struct seglist *segl;
    struct chart    *c;

    c = seg_parse(l, u, seg_combine);

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
            segl = get_segs_full(c);
        break;
        default:
            segl = get_segs_full(c);
        break;
    }

    chart_free(c);

    if (opt.score_arg == score_arg_best) {
        lexc_segl_score(segl, l, NULL, NULL, u);
    }

    return segl;
}


struct seglist * 
segment_lexicon(struct input *in, int idx)
{
    return lexicon_segment(L, in->u[idx].s);
}

void 
segment_lexicon_update(char *s, struct seglist *segl)
{
    return;
}

void 
segment_lexicon_cleanup()
{
    return;
}
