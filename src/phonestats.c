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

#include "phonestats.h"

struct phonestats *phonestats_init(size_t sigma_size, size_t maxng)
{
    struct phonestats *ps = malloc(sizeof *ps);

    ps->trie = trie_init(sigma_size);
    ps->mang = maxng;
    ps->ngcount = calloc(maxng, sizeof *ps->ngcount);

    return ps;
}

void phonestats_update(struct phonestats *ps, segunit_t *seq)
{
    size_t i = 0;
    segunit_t *v = seq;
    while (*v != 0) {
        trie_insert_span(ps->trie, v, 0, ps->maxng);
        i += 1;
        v += 1;
    }
}

size_t phonestats_freq_span(struct phonestats *ps, segunit_t *seq,
        size_t first, size_t length)
{
    struct trie_node *tn = trie_lookup_span(ps->trie, seq, first, length);

    if (tn != NULL) {
        return tn->count;
    } else {
        return 0;
    }

}

size_t phonestats_freq(struct phonestats *ps, segunit_t *seq)
{
    return phonestats_freq_span(ps, seq, 0, 0);
}



double phonestats_rfreq_span(struct phonestats *ps, segunit_t *seq,
        size_t first, size_t length)
{
    assert (ps->maxng >= length); 
    if (ps->ngcount[length - 1] == 0) {
        return 0.0;
    } else {
        return (phonestats_freq_span(ps, seq, first, length) + 1) /
               ps->ngcount[length - 1] + 1;
    }
}

double phonestats_rfreq(struct phonestats *ps, segunit_t *seq)
{
    size_t length = 0;
    segunit_t *tmp = seg;

    while(*tmp) {
        length += 1;
        tmp += 1;
    }
    return phonestats_rfreq_span(ps, seq, 0, length);
}

