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

#ifndef _LEXICON_H
#define _LEXICON_H       1  

#include <stdio.h>
#include <glib.h>

typedef struct cg_category {
    union {
    char       slash;            // slash operator, '\0' for basic cat.
    char       is_complex;       // just for convenience/readability 
    };
    struct cg_category  *res;    // res. cat., NULL for basic cat.
    struct cg_category  *arg;    // arg. cat., NULL for basic cat. 
    char                *str;    // this is also the key for the index 
    size_t              lex_freq;// this counts only lexical categories
    size_t              freq;    // this counts all
} cg_cat;                        

typedef struct cg_catlist {
    cg_cat            *cat;
    struct cg_catlist *next;
} cg_catlist;

typedef struct cg_lexitem {
    char        *pf;
    char        *lf;
    cg_cat      *cat;
    size_t      freq; // frequecy of this particular <pf,cat,lf>
} cg_lexi;

typedef struct cg_lexilist {
    cg_lexi             *lexi;
    struct cg_lexilist  *next;          // for simple traversal 
    struct cg_lexilist  *next_syn;      // next synonym 
    struct cg_lexilist  *next_hom;      // next homnym (or homophone) 
} cg_lexilist;

struct cg_listhead {
    cg_lexilist  *l;     // the list 
    size_t       n_typ;  // number of item types sharing same pf or lf
    size_t       n_tok;  // number of item tokens sharing same pf or lf
};

struct lex_stats {
    size_t n_typ, n_tok; // type and token counts of <pf,cat,lf> tuples
    size_t n_typ_pf;     // type count for pfs ( token counts are  )
    size_t n_typ_lf;     // type count for lfs ( the same as n_tok )
    size_t n_typ_cat;    // type count for all categories
    size_t n_typ_cat_lex;// type count for lexical categories
};                   

typedef struct cg_lexicon {
    GHashTable  *pfhash;
    GHashTable  *lfhash;
    GHashTable  *cathash;
    cg_catlist  *catl;
    cg_lexilist *ll;
    struct lex_stats *stats;
} cg_lexicon;

#define LEX_COMMENT ';'

cg_cat *cg_cat_new ();
void cg_cat_free (cg_cat *cat);

cg_lexi *cg_lexi_new ();
void cg_lexi_free (cg_lexi *lexi);

cg_lexicon *cg_lexicon_new();
void cg_lexicon_free(cg_lexicon *l);

cg_lexicon *cg_lexicon_read(FILE *fp);
void cg_lexicon_write(FILE *fp, cg_lexicon *l);
void cg_write_lexilist(FILE *fp, cg_lexilist *ll, char link);

cg_lexilist * cg_lexilist_new();
void cg_lexilist_free(cg_lexilist *ll, int free_item);


cg_lexicon *cg_lexicon_load(char *fname);
void cg_lexicon_save(char *fname, cg_lexicon *l);

struct cg_listhead *cg_lexicon_lookup_h(cg_lexicon *l, char *pf);
struct cg_listhead *cg_lexicon_lookup_lf_h(cg_lexicon *l, char *lf);
cg_lexilist *cg_lexicon_lookup(cg_lexicon *l, char *pf);
cg_lexilist *cg_lexicon_lookup_lf(cg_lexicon *l, char *pf);
cg_lexilist *
   cg_lexicon_lookup_full(cg_lexicon *l, char *pf, char *cat, char *lf);


cg_lexi *cg_lexicon_add(cg_lexicon *l, char *pf, char *cat, char *lf);
int cg_lexicon_addstr(cg_lexicon *l, char *lexistr);
void cg_lexicon_remove(cg_lexicon *l, cg_lexi *li);

cg_cat *cg_lexicon_addcat(cg_lexicon *l, char *catstr);


double cg_lexicon_get_rfreq_pf(cg_lexicon *l, char *pf);
size_t cg_lexicon_get_freq_pf(cg_lexicon *l, char *pf);

#endif /* _LEXICON_H */
