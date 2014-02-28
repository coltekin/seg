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
 * trie_int.c 
 *
 * Yet another trie (prefix tree) implementation. 
 *
 * This one is originally used for representing and processing 
 * multiple segmentation of a string (moderatly) efficiently.
 *
 * FIXME: not complete (partially migrated from mc2010 implementation).
 *
 */

struct ntree *
ntree_new(int len)
{
    struct ntree *ret = malloc(sizeof (struct ntree));
    ret->next = ret->children = NULL;
    ret->len = len;
    return ret;
}

void
ntree_free(struct ntree  *node)
{
    struct ntree   *tmp;

    tmp = node->children;
    while(tmp) {
        struct ntree   *tmp2;
        tmp2 = tmp->next;
        ntree_free(tmp);
        tmp = tmp2;
    }
    free(node);
}


struct segtrie *
segtrie_new()
{
    struct segtrie *ret = malloc(sizeof (struct segtrie));
    ret->max_depth = 0;
    ret->t = ntree_new(-1); // dummy root node
    return ret;
}

void
segtrie_free(struct segtrie *trie)
{
    ntree_free(trie->t);
    free(trie);
}

struct ntree *
ntree_find_child(struct ntree *node, int val)
{
    int     found = 0;
    struct ntree  *chi;

    if(node == NULL) return NULL;

    chi = node->children;
    while(chi) {
        if (chi->len == val) {
            found = 1;
            break;
        }
        chi = chi->next;
    }
    if(found) {
        return chi;
    } else {
        return NULL;
    }
}

void
segtrie_add(struct segtrie *trie, int seg[])
{
    struct ntree  *node = trie->t, *chi;
    int           i;


    if(seg[0] > trie->max_depth) {
        trie->max_depth = seg[0];
    }
    for(i = 1; i <= seg[0]; i++){
        chi = ntree_find_child(node, seg[i]);
        if (chi == NULL) {
            chi = ntree_new(seg[i]);
            chi->next = node->children;
            node->children = chi;
        }
        node = chi;
    }
}

void
wparse_get_seg(int seg[], cyk_chart_node *node, cyk_chart *chart)
{

    if(node->backL == NULL) {
        seg[seg[0]+1] = node->i + 1;
        seg[0]++;
        return;
    }
    if(node->backL == NULL) // this should not happen
        return;

    wparse_get_seg(seg, node->backL, chart);
    wparse_get_seg(seg, node->backR, chart);

}

struct segtrie *
wparse_get_segs(cyk_chart *chart)
{
    int             seg[chart->size + 1];
    struct segtrie  *ret = segtrie_new();

    if(chart->node[chart->size-1][0] != NULL) {
        cyk_chart_node *tmp = chart->node[chart->size-1][0];
        while(tmp) {
            seg[0] = 0;
            wparse_get_seg(seg, tmp, chart);
            segtrie_add(ret, seg);
            tmp = tmp->next;
        }
    }
    return ret;
}

struct segtrie *
wparse_get_segs_missing(cyk_chart *chart)
{
    int     i;
    int             *seg = malloc(sizeof(int) * chart->size + 1);
    struct segtrie  *ret = segtrie_new();

    for(i = 1; i < chart->size; i++) {
        cyk_chart_node *nodeL = chart->node[i-1][0],
                       *nodeR = chart->node[chart->size - i - 1][i];
        
        if(nodeL != NULL && nodeR == NULL) { 
            cyk_chart_node *tmp = nodeL;
            while(tmp) {
                seg[0] = 0;
                wparse_get_seg(seg, tmp, chart);
                seg[0] += 1;
                seg[seg[0]] = chart->size - i;
                segtrie_add(ret, seg);
                tmp = tmp->next;
            }
        } else if(nodeL == NULL && nodeR != NULL) {
            cyk_chart_node *tmp = nodeR;
            while(tmp) {
                seg[1] = 0;
                wparse_get_seg(seg + 1, tmp, chart);
                seg[0] = seg[1] + 1;
                seg[1] = i;
                segtrie_add(ret, seg);
                tmp = tmp->next;
            }
        } // else we do not care
    }
    free(seg);
    return ret;
}

struct seglist *
seglist_new()
{
    struct seglist *new;

    new = malloc(sizeof (struct seglist));
    new->nsegs = new->nalloc = 0;
    new->segs = NULL;
    return new;
}

void
seglist_free(struct seglist *segl)
{
    int i;

    for(i=0; i < segl->nsegs; i++) {
        free(segl->segs[i]);
    }
    free(segl->segs);
    free(segl);
}


void
seglist_add(struct seglist *segl, int *seg)
{
    int i;
    int *newseg = NULL;
    
    newseg = malloc((*seg + 1) * sizeof(int));

    for(i=0; i <= seg[0]; i++) {
        newseg[i] = seg[i];
    }

    segl->nsegs++;

    if(segl->nsegs >= (segl->nalloc / sizeof (int *))) {
        segl->segs = realloc(segl->segs, BUFSIZ);
        segl->nalloc += BUFSIZ;
    }

    segl->segs[segl->nsegs - 1] = newseg;
}


void
ntree_traverse_r(struct ntree   *node, 
                 int            *seg, 
                 int            depth,
                 struct seglist *segs)
{
    struct ntree   *tmp = node;

    if(tmp == NULL){
        seglist_add(segs, seg);
        return;
    }
    while(tmp) {
        seg[0] = depth + 1;
        seg[seg[0]] = tmp->len;
        ntree_traverse_r(tmp->children, seg, depth + 1, segs);
        tmp = tmp->next;
    }
}

struct seglist *
wparse_segtrie_unpack(struct segtrie *trie)
{
    struct seglist *ret = seglist_new();
    int    *seg = malloc((trie->max_depth + 1) * sizeof (int));
    seg[0] = 0;

    ntree_traverse_r(trie->t->children, seg, 0, ret);

    free(seg);
    return ret;
}

struct seglist *
wparse_get_seglist(cyk_chart *chart)
{
    struct seglist *segs;
    struct segtrie *strie;

    strie = wparse_get_segs(chart);
    if(strie->max_depth != 0) {
        segs = wparse_segtrie_unpack(strie);
        segtrie_free(strie);
        return segs;
    } else {
        segtrie_free(strie);
        return NULL;
    }
    
}
