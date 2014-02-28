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

#ifndef _TRIE_INT_H
#define _TRIE_INT_H       1

struct ntree { //simple n-ary tree
    int             len;        // length of the segment
    struct ntree    *next;      // siblings of this node
    struct ntree    *children;  // pointer to the first child
};

struct trie_int {
    int             max_depth;
    struct ntree    *t;
};

/* 
 * this structure is returned when an `unpacked' version 
 * of the trie is requested
*/
struct trie_int_list { // technically an array, not a list
    int nmemb;
    int nalloc;
    int **memb;
};


struct ntree *ntree_new(int len);
void ntree_free(struct ntree  *node);
struct segtrie *segtrie_new();
void segtrie_free(struct segtrie *trie);
struct segtrie *wparse_get_segs(cyk_chart *chart);
struct segtrie *wparse_get_segs_missing(cyk_chart *chart);
struct seglist *seglist_new();
void seglist_free(struct seglist *segl);
void seglist_add(struct seglist *segl, int seg[]);
struct seglist *wparse_segtrie_unpack(struct segtrie *trie);

#endif // _TRIE_INT_H
