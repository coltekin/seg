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

#ifndef _PHONSTATS_H
#define _PHONSTATS_H 1

#include <stddef.h>
#include <glib.h>
#include "prob_dist.h"

#define BOW_CH  '<'
#define EOW_CH  '>'

enum {  // to map array inices to meaningful names
    NG_UNIGRAM=0,
    NG_BIGRAM,
    NG_TRIGRAM,
    NG_4GRAM,
    NG_5GRAM,
    NG_6GRAM,
};

enum prob_opts {
    SMOOTH_NONE,
    SMOOTH_ADD1,
    SMOOTH_WB,
};

/*
 * Notes on phonstats structure:
 *
 * Hash tables: this structure keeps max_ng + 1 hash tables. The first
 * one hash[0] indexes all the ngrams. The other hash tables keep all
 * the ngrams of respective size. These are mainly used for
 * enumerating all possible ngrams for a given n.
 *
 */
struct phonstats {
    size_t      max_ng;  
    size_t      n_updt; // this is the number of boundaries (< and >) given
    size_t      *n_tok; // arrays of max_ng
    size_t      *n_typ;
    struct prob_dist **st; // mean&variance for each nglen
    size_t      *nalloc; // internal use, to alloc/realloc memory
    char        ***ngstr;
    GHashTable  *hash;
};

struct phonstats * phonstats_new(size_t max_ng, char *phon_list);
struct phonstats * phonstats_new_st(size_t max_ng, char *phon_list);
void phonstats_free(struct phonstats *ps);
// void inc_phonfreq(struct phonstats *ps, unsigned char ph);

size_t phonstats_freq_p(struct phonstats *ps, char ch);

size_t phonstats_freq_ng(struct phonstats *ps, char *ng);

double phonstats_rfreq_p(struct phonstats *ps, unsigned char ch);
double phonstats_rfreq_p2(struct phonstats *ps, unsigned char ch);
double phonstats_rfreq_ng(struct phonstats *ps, char *ch);
void phonstats_update(struct phonstats *ps, char *s);

double
phonstats_P(struct phonstats *ps, char *ng, int options);

void phonstats_dump(struct phonstats *ps);

void phonstats_update_from_file(struct phonstats *ps, char *fname);

double phonstats_nglen_mean(struct phonstats *ps, int nglen);
double phonstats_nglen_sd(struct phonstats *ps, int nglen);
double phonstats_freq_z(struct phonstats *ps, char *ng);

void phonstats_copy(struct phonstats *dst, struct phonstats *src);

#endif // _PHONSTATS_H
