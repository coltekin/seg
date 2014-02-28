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

#include "lexicon.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "strutils.h"

/*
 * cg_lexicon_new()
 *
 * Allocate memory for a new lexicon structure, and initialize.
 *
 */
cg_lexicon *
cg_lexicon_new()
{
    cg_lexicon *new;

    new = malloc(sizeof(*new));

    new->pfhash = g_hash_table_new(g_str_hash, g_str_equal);
    new->lfhash = g_hash_table_new(g_str_hash, g_str_equal);
    new->cathash = g_hash_table_new(g_str_hash, g_str_equal);
    new->stats = malloc (sizeof(*new->stats));
    new->stats->n_typ = 0;
    new->stats->n_tok = 0;
    new->stats->n_typ_pf = 0;
    new->stats->n_typ_lf = 0;
    new->stats->n_typ_cat = 0;
    new->stats->n_typ_cat_lex = 0;
    new->catl = NULL;
    new->ll = NULL;

    return new;
}

/*
 * cg_lexi_new()
 *
 * Allocate memory for a new lexitem and set all pointers to NULL.
 */
cg_lexi *
cg_lexi_new()
{
    cg_lexi *new;
    new = malloc (sizeof(*new));
    new->pf = NULL;
    new->lf = NULL;
    new->cat = NULL;
    new->freq = 0;
    return new;
}

void
cg_lexi_free(cg_lexi *li)
{
/*
    if(li->pf) free(li->pf);
    if(li->lf) free(li->lf);
        if(li->cat) cg_cat_free(li->cat);
*/
    free(li);
}

/*
 * cg_lexitem_new()
 *
 * Allocate memory for a new category structure and set all pointers to NULL.
 */
cg_cat *
cg_cat_new()
{
    cg_cat *ret = malloc (sizeof(cg_cat));
    ret->slash = '\0';
    ret->str = NULL;
    ret->res = NULL;
    ret->arg = NULL;
    ret->freq = 0;
    return ret;
}

void
cg_cat_free(cg_cat *cat)
{
        if(cat->str) free(cat->str);
        free(cat);
}

/*
 * cg_lexilist_new()
 *
 * Allocate memory for a new lexilist and set all pointers to NULL.
 */
cg_lexilist *
cg_lexilist_new()
{
    cg_lexilist *ret = malloc (sizeof(cg_lexilist));
    ret->lexi = NULL;
    ret->next = ret->next_syn = ret->next_hom = NULL;
    return ret;
}

/*
 * cg_lexilist_free(cg_lexilist *ll, int free_item)
 *
 * free's the memory allocated for lexitem list ll. if the paremeter 
 * free_item is 1, the associated lexical item is also freed. Otherwise
 * only the list is destoyed.
 */
void
cg_lexilist_free(cg_lexilist *ll, int free_item)
{
    cg_lexilist *tmp1, *tmp2;
    tmp1 = ll;
    while(tmp1) {
        tmp2 = tmp1->next;
        if (free_item) 
            cg_lexi_free(tmp1->lexi);
        free(tmp1);
        tmp1 = tmp2;
    }
}

void
cg_catlist_free(cg_catlist *cl, int free_cat)
{
    cg_catlist *tmp1, *tmp2;
    tmp1 = cl;
    while(tmp1) {
        tmp2 = tmp1;
        tmp1 = tmp1->next;
        if (free_cat) cg_cat_free(tmp2->cat);
        free(tmp2);
    }
}

void 
cg_lexicon_free (cg_lexicon *l)
{
    GHashTableIter iter;
    gpointer key, val;

    cg_lexilist_free(l->ll, 1);
    cg_catlist_free(l->catl, 1);

    g_hash_table_iter_init (&iter, l->pfhash);
    while (g_hash_table_iter_next (&iter, &key, &val)) {
        if(key) free(key);
        if(val) free(val);
    }
    g_hash_table_iter_init (&iter, l->lfhash);
    while (g_hash_table_iter_next (&iter, &key, &val)) {
        if(key) free(key);
        if(val) free(val);
    }

    g_hash_table_destroy(l->cathash);
    g_hash_table_destroy(l->pfhash);
    g_hash_table_destroy(l->lfhash);
    free(l->stats);
    free(l);
    return;
}

inline char *
pf_normalize(char *pfs)
{
    char *s = strdup(pfs);

    str_strip(s, " \t\n");
    return s;
}

inline char *
lf_normalize(char *lfs)
{
    if(lfs == NULL) 
        return NULL;
    char *s = strdup(lfs);
    return str_rmchs(s, " \t", NULL);
}

inline char *
cat_normalize(char *cats)
{
    char *s = strdup(cats);

    str_rmchs(s, " \t", NULL);

    // remove parantheses at the beginning and the end
    while(*s == '(') {
        char *tmp = s;
        int pcount = 0;

        while(*(tmp+1)) {
            if(*tmp == '(') pcount++;
            if(*tmp == ')') pcount--;
            if(pcount == 0) break;
            tmp++;
        }
        if(pcount == 1) {
            tmp = s;
            while(*tmp){
                *tmp = *(tmp+1);
                tmp++;
            }
            *(tmp-2) = '\0';
        } else if (pcount == 0) {
            break;
        } else {
            fprintf(stderr, "Catstr %s has unbalanced parantheses\n", s);
        }
    }
    return s;
}

cg_cat *
cg_lexicon_addcat_f(cg_lexicon *l, char *catstr, size_t freq)
{
    cg_cat      *cat;
    cg_catlist  *catl;
    int         i, pcount;
    size_t      n;
    char        *tmp = cat_normalize(catstr);;

    cat = g_hash_table_lookup(l->cathash, tmp);
    if(cat) {/* we already have the category in the lexicon */
        cat->freq += freq;
        free(tmp);
        return cat;
    }

    pcount = 0;
    cat = cg_cat_new();
    cat->freq += freq;
    l->stats->n_typ_cat += 1;
    if (freq > 0 ) {
        l->stats->n_typ_cat_lex += 1;
    }
    cat->str = tmp;
    n = strlen(cat->str);

    for(i=0;i<n;i++){ /*find the top level slash*/
        if(cat->str[i] == '(')
            pcount++;
        else if (cat->str[i] == ')')
            pcount--;
        else if (pcount == 0 && (cat->str[i] == '/' || cat->str[i] == '\\'))
            break;
    }

    if(i==n){ /* no slash: basic category*/
        cat->slash = '\0';
        cat->res = cat->arg = NULL;
    } else { /* complex category */
        char        *res_str, *arg_str;

        res_str = malloc(i+1);
        arg_str = malloc(n-i+1);
        strncpy(res_str, cat->str, i); res_str[i] = '\0';
        strncpy(arg_str, cat->str+i+1, n-i); arg_str[n-i] = '\0';

        cat->res = cg_lexicon_addcat_f(l, res_str, 0);
        free(res_str);

        cat->arg = cg_lexicon_addcat_f(l, arg_str, 0);
        free(arg_str);

        cat->slash = cat->str[i];
    }

    g_hash_table_insert (l->cathash, cat->str, cat);

    catl = malloc(sizeof (cg_catlist));
    catl->next = l->catl;
    catl->cat = cat;
    l->catl = catl;

    return cat;
}

cg_cat *
cg_lexicon_addcat(cg_lexicon *l, char *catstr)
{
    return cg_lexicon_addcat_f(l, catstr, 1);
}

/* 
 * Functions for adding a lexical item to the lexicon
 */

void 
inc_freq(cg_lexicon *l, cg_lexi *lexi, size_t freq)
{
    char *key = NULL;
    char *lfkey = NULL;
    struct cg_listhead *val = NULL;

    lexi->freq += freq;
    l->stats->n_tok += freq;
    lexi->cat->freq += freq;
    g_hash_table_lookup_extended(l->pfhash, lexi->pf, 
                             (gpointer *)&key, (gpointer *)&val);
    val->n_tok += freq;

    lfkey = lexi->lf ? lexi->lf : ":";
    g_hash_table_lookup_extended(l->pfhash, lfkey,
                             (gpointer *)&key, (gpointer *)&val);
    val->n_tok += freq;
    
}

cg_lexi *
cg_lexicon_add_f(cg_lexicon *l, char *pf, char *cat, char *lf, size_t freq)
{
    char *key = NULL;
    struct cg_listhead *val = NULL;
    cg_lexi *lexi = NULL;
    cg_lexilist *lexl = NULL;
    char *pfs = pf_normalize(pf);
    char *lfs = lf_normalize(lf);
    char *cats = cat_normalize(cat);
    char *lfkey;
    
    assert(l != NULL);

    lexl = cg_lexicon_lookup_full(l, pfs, cats, lfs);
    if (lexl != NULL) {
        lexi = lexl->lexi; 
        inc_freq(l, lexi, freq);
        cg_lexilist_free(lexl, 0);
        free(pfs);
        free(cats);
        if (lfs) free(lfs);
        return lexi;
    }
    /* Exact lexical item <pf,cat,lf> does not exist */

    lexi = cg_lexi_new();
    lexi->freq += freq;
    l->stats->n_tok += freq;
    ++l->stats->n_typ;
    lexl = cg_lexilist_new();
    lexl->lexi = lexi;
    lexl->next = l->ll;
    l->ll = lexl;

    lexi->cat = cg_lexicon_addcat_f(l, cats, freq);
    free(cats);

    
    if(g_hash_table_lookup_extended(l->pfhash, pfs, 
                             (gpointer *)&key, (gpointer *)&val)){
        lexi->pf = key;
        lexl->next_hom = val->l;
        val->l = lexl;
        val->n_tok += freq;
        ++val->n_typ;
        free(pfs);
    } else {
        lexi->pf = pfs;
        val = malloc (sizeof (*val));
        val->n_typ = 1;
        ++l->stats->n_typ_pf;
        val->n_tok = freq;
        val->l = lexl;
        g_hash_table_insert (l->pfhash, lexi->pf, val);
    }
    
    // key ":" collects all the lexial items with null LF
    lfkey = lfs ? lfs : ":";

    if(g_hash_table_lookup_extended(l->lfhash, lfkey, 
                             (gpointer *)&key, (gpointer *)&val)){
        lexi->lf = lfs ? key : NULL;
        lexl->next_syn = val->l;
        val->l = lexl;
        ++val->n_typ;
        val->n_tok += freq;
        lfkey = key;
        if (lfs) free(lfs);
    } else {
        lexi->lf = lfs; 
        if(lfs == NULL) 
            lfkey = strdup(":");
        val = malloc (sizeof (*val));
        val->n_typ = 1;
        ++l->stats->n_typ_lf;
        val->n_tok = freq;
        val->l = lexl;
        g_hash_table_insert (l->lfhash, lfkey, val);
    }

    return lexi;
}

cg_lexi *
cg_lexicon_add(cg_lexicon *l, char *pf, char *cat, char *lf)
{
    return cg_lexicon_add_f(l, pf, cat, lf, 1);
}

int
cg_lexicon_addstr(cg_lexicon *l, char *str)
{
    char *pf, *cat, *lf, *tmp;
    size_t freq;

    pf = str;
    tmp = strstr(str, ":=");
    if(!tmp) return 0;

    *tmp = '\0';
    cat = tmp + 2;

    tmp = strchr(cat, ':');
    if(!tmp) {
        lf = NULL;
        freq = 1;
    } else {
        *tmp = '\0';
        lf = tmp + 1;

        str_rmchs(lf, " \t", NULL);
        tmp = strchr(lf, ':');
        if(*lf == ':' || !strlen(lf))
            lf = NULL;
        if(!tmp) {
            freq = 1;
        } else {
            *tmp = '\0';
            freq = strtol(tmp + 1, NULL, 0);
            assert(freq > 0);
        }
    }

    cg_lexicon_add_f(l, pf, cat, lf, freq);

    return 1;
}

/* cg_lexicon_remove(cg_lexicon *l, cg_lexi *li) 
 *
 * remove the li from the lexicon. 
 * we only compare the address of the li, hence, the li passed
 * has to be in the lexicon (an externally created li with the 
 * same content is not removed). 
 *
 */
void
cg_lexicon_remove(cg_lexicon *l, cg_lexi *li)
{
    struct cg_listhead *pf_head = cg_lexicon_lookup_h(l, li->pf);
    struct cg_listhead *lf_head = cg_lexicon_lookup_lf_h(l, li->lf);

    if (pf_head != NULL && lf_head != NULL) {
        cg_lexilist *pf_parent = pf_head->l,
                    *lf_parent = lf_head->l,
                    *ll = l->ll;
            // remove the ->next link
            if (ll->lexi == li) { // first item in the list
                l->ll = ll->next;
            } else {
               cg_lexilist *ll_parent = l->ll; 
                while (ll_parent->next && ll_parent->next->lexi != li)
                    ll_parent = ll_parent->next;
                assert(ll != NULL);
                ll = ll_parent->next;
                ll_parent->next = ll->next;
            }

            // remove the ->next_hom link
            if (pf_parent == ll) { // first in pf list
                if (ll->next_hom == NULL) { // the only one
                    g_hash_table_remove(l->pfhash, li->pf);
                    free(li->pf);
                    free(pf_head);
                    --l->stats->n_typ_pf;
                } else  {
                    pf_head->l = ll->next_hom;
                }
            } else {
                while (pf_parent->next_hom &&  pf_parent->next_hom != ll)
                    pf_parent = pf_parent->next_hom;
                assert (pf_parent->next_hom != NULL);
                pf_parent->next_hom = ll->next_hom;
            }

            // remove the ->next_syn link
            if (lf_parent == ll) {
                if (ll->next_syn == NULL) {
                    if(li->lf == NULL) {
                        g_hash_table_remove(l->lfhash, ":");
                    } else {
                        g_hash_table_remove(l->lfhash, li->lf);
                        free(li->lf);
                    }
                    free(lf_head);
                    --l->stats->n_typ_pf;
                } else  {
                    lf_head->l = ll->next_syn;
                }
            } else {
                while (lf_parent->next_syn &&  lf_parent->next_syn != ll)
                    lf_parent = lf_parent->next_syn;
                assert(lf_parent->next_syn != NULL);
                lf_parent->next_syn = ll->next_syn;
            }

            l->stats->n_tok -= li->freq;
            l->stats->n_typ -= 1;
            cg_lexi_free(ll->lexi);
            free(ll);
    }
}

/* 
 * lookup functions
 */

/*
 * cg_lexicon_lookup(cg_lexicon *l, char *pf)
 *
 *      returns the head of the lexilist associated with the pf.
 *      one needs to follow ->next_hom links to traverse this list.
 *      the pointer returned is data stored in the lexicon, and
 *      should NOT be freed.
 *
 *      _h version returns the dummy listhead structure.
 */

inline struct cg_listhead *
cg_lexicon_lookup_h(cg_lexicon *l, char *pf)
{
    char        *pfn = pf_normalize(pf);
    struct cg_listhead *lh;

    lh = g_hash_table_lookup(l->pfhash, pfn);
    free(pfn);
    return lh;
}


cg_lexilist *
cg_lexicon_lookup(cg_lexicon *l, char *pf)
{
    struct cg_listhead *lh = cg_lexicon_lookup_h(l,pf);

    return lh ? lh->l : NULL;
}

/*
 * cg_lexicon_lookup_lf(cg_lexicon *l, char *lf)
 *
 *      returns the head of the lexilist associated with the lf.
 *      one needs to follow ->next_syn links to traverse this list.
 *      the pointer returned is data stored in the lexicon, and
 *      should NOT be freed.
 *
 *      _h version returns the dummy listhead structure.
 */

inline struct cg_listhead *
cg_lexicon_lookup_lf_h(cg_lexicon *l, char *lf)
{
    char        *lfn = lf_normalize(lf);
    struct cg_listhead *lh;

    if(lfn == NULL)  {
        lh = g_hash_table_lookup(l->lfhash, ":");
    } else {
        lh = g_hash_table_lookup(l->lfhash, lfn);
        free(lfn);
    }
    return lh; 
}
 
cg_lexilist *
cg_lexicon_lookup_lf(cg_lexicon *l, char *lf)
{
    return (cg_lexicon_lookup_lf_h(l,lf))->l;
}

/*
 * cg_lexicon_lookup_full(cg_lexicon *l, char *pf, char *catstr, char *lf)
 *
 *      returns the head of the lexilist containing items maching all
 *      given arguments. 
 *
 *      The empty string is interpreted as "any" lf or cat. 
 *
 *      The pointer returned is a copy with ->next_hom and 
 *      ->next_syn pointers set to NULL. 
 *      one needs to follow ->next links to traverse this list.
 *
 *      This function returns a new list that points to the lexical 
 *      items in the lexicon. The caller should free it with 
 *      cg_lexilist_free(list, 0).
 */

cg_lexilist *
cg_lexicon_lookup_full(cg_lexicon *l, char *pf, char *catstr, char *lf)
{
    char        *pfn = pf_normalize(pf),
                *lfn = lf_normalize(lf), 
                *catn = cat_normalize(catstr);
    int         check_lf = (lfn != NULL && strlen(lfn) != 0),
                check_cat = (strlen(catn) != 0);
    cg_lexilist *ret, *lret, *pnew, *pold;
    struct cg_listhead *lh;

    lh = g_hash_table_lookup(l->pfhash, pfn);

    free(pfn);

    if(lh == NULL) {
        if (lfn) free(lfn);
        if (catn) free(catn);
        return NULL;
    }

    lret = lh->l;
    pold = lret;
    ret = NULL;

    do {
        int lf_match,
           cat_match;
        
        cat_match = (check_cat) ? !strcmp(pold->lexi->cat->str, catn) : 1;

        if(lfn == NULL || pold->lexi->lf == NULL) {
            lf_match = (lfn == pold->lexi->lf);
        } else {
            lf_match = (check_lf) ? !strcmp(pold->lexi->lf, lfn) : 1;
        }

        if(lf_match && cat_match) {
            pnew = cg_lexilist_new();
            pnew->lexi = pold->lexi;
            pnew->next = ret;
            ret = pnew;
        }
        pold = pold->next_hom;
    } while(pold);

    if (catn) free(catn);
    if (lfn) free(lfn);

    return ret; 
}

size_t 
cg_lexicon_get_freq_pf(cg_lexicon *l, char *pf)
{
    char *key = NULL;
    struct cg_listhead *val = NULL;
    char *pfn = pf_normalize(pf);

    g_hash_table_lookup_extended(l->pfhash, pfn,
                                 (gpointer *)&key, (gpointer *)&val);
    free(pfn);
    return val ? val->n_tok : 0;
}

double 
cg_lexicon_get_rfreq_pf(cg_lexicon *l, char *pf)
{
    char *key = NULL;
    struct cg_listhead *val = NULL;
    char *pfn = pf_normalize(pf);

    g_hash_table_lookup_extended(l->pfhash, pfn,
                                 (gpointer *)&key, (gpointer *)&val);
    free(pfn);
    if (val) {
        return (double) val->n_tok / (double) l->stats->n_tok ;
    } else {
        return 0.0;
    }
}


/* 
 * IO functions
 */

void 
cg_write_lexilist(FILE *fp, cg_lexilist *ll, char link)
{
    while(ll) {
        if(ll->lexi->lf) {
            if(ll->lexi->freq > 1) {
                fprintf(fp, "%s := %s : %s : %zu\n", ll->lexi->pf,
                                                     ll->lexi->cat->str,
                                                     ll->lexi->lf,
                                                     ll->lexi->freq);
            } else {
                fprintf(fp, "%s := %s : %s\n", ll->lexi->pf,
                                               ll->lexi->cat->str,
                                               ll->lexi->lf);
            }
        } else {
            if(ll->lexi->freq > 1) {
                fprintf(fp, "%s := %s :: %zu\n", ll->lexi->pf,
                                                 ll->lexi->cat->str,
                                                 ll->lexi->freq);
            } else {
                fprintf(fp, "%s := %s\n", ll->lexi->pf,
                                          ll->lexi->cat->str);
            }
        }
        switch(link){
            case 's': ll = ll->next_syn;
            break;
            case 'h': ll = ll->next_hom;
            break;
            default: ll = ll->next;
        }
    }
}

void
cg_lexicon_write(FILE *fp, cg_lexicon *l)
{
    GHashTableIter iter;
    char    *key = NULL;
    struct cg_listhead *lh;
    cg_cat *cat;

    fprintf(fp, "%c Lexicon types/tokens: %zu/%zu\n", LEX_COMMENT, 
            l->stats->n_typ, l->stats->n_tok);
    fprintf(fp, "%c pf/lex_cat:cat/lf type counts: %zu/%zu:%zu/%zu\n", 
            LEX_COMMENT, l->stats->n_typ_pf, 
            l->stats->n_typ_cat_lex, l->stats->n_typ_cat,
            l->stats->n_typ_lf);

    fprintf(fp, "%c \n", LEX_COMMENT); 
    fprintf(fp, "%c type/token counts for each PF...\n", LEX_COMMENT);
    g_hash_table_iter_init (&iter, l->pfhash);
    while (g_hash_table_iter_next (&iter, (gpointer*) &key, (gpointer*) &lh)) {
        fprintf(fp, "%cPF `%s': %zu/%zu\n", 
                LEX_COMMENT, key, lh->n_typ, lh->n_tok);
    }
    fprintf(fp, "%c \n", LEX_COMMENT); 
    fprintf(fp, "%c Category table...\n", LEX_COMMENT);
    g_hash_table_iter_init (&iter, l->cathash);
    while (g_hash_table_iter_next (&iter, (gpointer*) &key, (gpointer*) &cat)) {
        fprintf(fp, "%cCAT  `%s': %zu\n", LEX_COMMENT, key, cat->freq);
    }
    fprintf(fp, "%c \n", LEX_COMMENT); 
    fprintf(fp, "%c type/token counts for each LF...\n", LEX_COMMENT);
    g_hash_table_iter_init (&iter, l->lfhash);
    while (g_hash_table_iter_next (&iter, (gpointer*) &key, (gpointer*) &lh)) {
        fprintf(fp, "%cLF  `%s': %zu/%zu\n", LEX_COMMENT, key, 
            lh->n_typ, lh->n_tok);
    }
    fprintf(fp, "%c \n", LEX_COMMENT); 
    fprintf(fp, "%c \n", LEX_COMMENT); 

    cg_write_lexilist(fp, l->ll, 'l');
}

cg_lexicon *
cg_lexicon_read(FILE *fp)
{
    cg_lexicon  *lexicon;
    int         lineno = 0;

    lexicon = cg_lexicon_new(); 
    while(!feof(fp)){
        char    *linebuf=NULL;

        lineno++;
        fscanf(fp, "%m[^\n]\n", &linebuf);

        /* skip the comments */
        if(*linebuf == LEX_COMMENT) {
            free(linebuf);
            continue;
        }
        if(!cg_lexicon_addstr(lexicon, linebuf)){
            fprintf(stderr, "Warning: ignoring line %d: `%s'\n",
                    lineno, linebuf);
        }
        free(linebuf);
    }

    return lexicon;
}

cg_lexicon *
cg_lexicon_load(char *fn)
{
    FILE    *fp;
    cg_lexicon *L;

    if(!strcmp(fn, "-")) {
        fp = stdin;
    } else {
        fp = fopen(fn, "r");
    }
    if(!fp) {
        fprintf(stderr, "Error opening file `%s' for reading.", fn);
        exit(-1);
    }

    L = cg_lexicon_read(fp);
    fclose(fp);
    return L;
}

void 
cg_lexicon_save(char *fn, cg_lexicon *l)
{
    FILE    *fp;

    if(!strcmp(fn, "-")) {
        fp = stdout;
    } else {
        fp = fopen(fn, "w");
    }
    if(!fp) {
        fprintf(stderr, "Error opening file `%s' for writing.", fn);
        exit(-1);
    }

    cg_lexicon_write(fp, l);

    fclose(fp);
}

