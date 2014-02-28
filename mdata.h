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

#ifndef _MDATA_H
#define _MDATA_H 1
#include "cmdline.h"
#include "options.h"
#include "lexicon.h"


struct minfo;
enum m_id;

struct mdata {
    struct minfo *info;
    char *s;        //pointer to the (utterance) string 
//    char *l;        //left  phoneme context (can be NULL)
//    char *r;        //right phoneme context (can be NULL)
    short len_l;    //length of left phoneme context   (-1 if irrelevant)
    short len_r;    //lenght of right phoneme context  (... 0 if unset  )
    double w_l;     //weight for wmv, normally w_l == w_r
    double w_r;     //different if contribution of l&r are counted individually
    struct phonstats *ps;
    cg_lexicon *L;  //these two are used by lexicon based seg.
    struct ctxlex *cL;  //these two are used by lexicon based seg.
    struct chart *c;//chart can be null
};

struct mdlist {
    struct mdata **md;
    size_t n;
    size_t nalloc;
};



struct mdata * mdata_new();
struct mdata * mdata_new_full(enum m_id mid, char *s, short len_l, short len_r);
void mdata_free(struct mdata *md);
struct mdlist * mdlist_new();
void mdlist_add (struct mdlist *mdl, struct mdata *md);
void mdlist_free(struct mdlist *mdl);

#endif // _MDATA_H
