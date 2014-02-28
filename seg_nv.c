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

#include "seg_nv.h"
#include "seg.h"
#include "segparse.h"
#include "lex.h"
#include <assert.h>
#include "packed_chart.h"
#include "strutils.h"

static cg_lexicon *L;


void 
segment_nv_init(struct input *in)
{
    if(opt.inlex_given){
        L = cg_lexicon_load(opt.inlex_arg);
    } else {
        L = cg_lexicon_new();
    }
}

struct seglist * 
segment_nv(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    int i;
    int len = strlen(u);
    struct seglist *segl = seglist_new();
    short  unsigned seg[len + 1];
    struct chart *c;

    c = seg_parse(L, u, seg_combine);
//    chart_write(stdout, c);

    seg[0] = 0;

/*
    segl = get_segs_full(c);

    if (segl->nsegs == 1 && (segl->segs[0] == NULL || segl->segs[0][0] == 0)) {
        cg_lexicon_add(L, u, "x", NULL);
    } else {
        int j;
        int start = 0;
        for (j = 1; j <= segl->segs[0][0]; j++) {
            int len = segl->segs[0][0] - start;
            char tmp[len + 1];
            str_rangecpy(tmp, u, start, len);
            cg_lexicon_add(L, tmp, "x", NULL);
        }
    }
*/

 //   printf("%s:\n", u);
    for (i = 1; i < len - 1; i++) {
        if(words_before(L, c, i, 0) ||  words_after(L, c, i, 0)) {
            ++seg[0];
            seg[seg[0]] = i;
        }
    }
    seglist_add(segl, seg);
//    printf("\n");

//    printf ("%s ", u);
//    seglist_write(stdout, segl);
    

    chart_free(c);
    segment_nv_update(u, segl);
    return segl;
}

void 
segment_nv_update(char *s, struct seglist *segl)
{
    char tmp[strlen(s)];

    if (segl->segs[0] == NULL || segl->segs[0][0] == 0) {
        cg_lexicon_add(L, s, "x", NULL);
    } else {
        int i;
        int start =0;
        for (i = 1; i <= segl->segs[0][0]; i++) {
            int len = segl->segs[0][i] - start;
            str_rangecpy(tmp, s, start, len);
            cg_lexicon_add(L, tmp, "x", NULL);
            start = segl->segs[0][i];
        }
    }

    return;
}

void 
segment_nv_cleanup()
{
    return;
}
