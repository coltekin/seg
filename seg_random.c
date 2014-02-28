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
 * Random segmentation
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "seg.h"
#include "io.h"
#include "seglist.h"

static double rate = 0.5;

void 
segment_random_init(struct input *in)
{
    rate = opt.random_rate_arg;
    if (opt.random_seed_given) {
        srand(opt.random_seed_arg);
    } else {
        srand((unsigned int)time(NULL));
    }
}

struct seglist * 
segment_random(struct input *in, int idx)
{
    char *u = in->u[idx].s;
    struct seglist *segl = seglist_new();
    int  len = strlen(u) - 1;
    unsigned short seg[len + 1];
    int i = 1, j;

    seg[0] = 0;

    for (j = 1; j <= len; j++) {
        if (((double)rand() / (double) RAND_MAX) < rate) {
            seg[i] = j;
            ++seg[0];
            ++i;
        }
    }

    seglist_add(segl, seg);

    return segl;
}

void 
segment_random_update(char *s, struct seglist *segl)
{
    return;
}

void 
segment_random_cleanup()
{
    return;
}
