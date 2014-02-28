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

#include "packed_chart.h"
#include "lexicon.h"
#include "strutils.h"
#include "stack.h"
#include "cclib_debug.h"

struct chart *
chart_new(unsigned short size)
{
    struct chart   *ret;
    int            i, j;

    ret = malloc(sizeof (cyk_chart));
    ret->node = malloc (size * sizeof (cyk_chart_node *));
    ret->input = malloc (size * sizeof (char *));
    ret->size = size;

    for(i = 0; i < size; i++) { 
        ret->node[i] = malloc ((size - i) * sizeof (cyk_chart_node));
        for(j=0; j < size - i; j++){
            ret->node[i][j] = NULL;
        }
        ret->input[i] = NULL;
    }
    return ret;
}

static void
chart_node_free(struct chart_node *node)
{
    struct backlink *b = node->back;
    struct backlink *tmp;
    while(b != NULL) {
        tmp = b->next;
        free(b);
        b = tmp;
    }
    free(node);
}

void 
chart_free(struct chart *chart)
{
    int i, j;

    if(chart == NULL) 
        return;

    for(i=0; i < chart->size; i++) {
        for(j=0; j < chart->size - i; j++) {
            struct chart_node *tmp = chart->node[i][j]; 
            struct chart_node *tmp2;
            while(tmp) {
                tmp2 = tmp->next;
                chart_node_free(tmp);
                tmp = tmp2;
            }
        }
        free(chart->node[i]);
        if(chart->input[i]) free(chart->input[i]);
    }
    free(chart->input);
    free(chart->node);
    free(chart);
    
}

/*
 * chart_node_ad() 
 * 
 * try to add a node to the chart
 * if the node matches with an existing entry the function 
 * does nothing, returns a pointer to existing node. otherwise,
 * it creates appropriate structures (either adding a complete 
 * node or adding backlinks to an existing node) and returns 
 * the pointer to the newly created or modified node.
 *
 */
struct chart_node *
chart_node_add(struct chart *c,
               unsigned short i, unsigned short j,
               cg_cat *cat, 
               struct chart_node *bL, 
               struct chart_node *bR) 
{
    struct chart_node *node = c->node[i][j];

    while(node != NULL && node->cat != cat) {
        node = node->next;
    }

    if(node != NULL) { // we have the category
        struct backlink *b = node->back;
        while(b != NULL && 
              !(b->L == bL && b->R == bR)) {
            b = b->next;
        }
        if(b == NULL) { // we don't have the cat with same backlinks
            b = malloc (sizeof (*b));
            b->next = node->back;
            b->L = bL;
            b->R = bR;
            node->back = b;
        } // else (if we have <cat,bL,bR> just return the node
    } else {
        node = malloc (sizeof (*node));
        if(bL != NULL) {
            node->back = malloc (sizeof (*node->back));
            node->back->L = bL;
            node->back->R = bR;
            node->back->next = NULL;
        } else {
            node->back = NULL;
        }
        node->cat = cat;
        node->sp_start = j;
        node->sp_len = i + 1;
        node->next = c->node[i][j];
        c->node[i][j] = node;
    }
    return node;
}


/* 
 * chart_write(FILE *fp, struct chart *chart)
 *
 * write the chart to given file (mostly for debugging purposes)
 */
void 
chart_write(FILE *fp, struct chart *chart)
{
    if (chart == NULL) {
       fprintf(stderr,"Chart is NULL\n");   
       return;
    }

    int i, j;
    int max_len = 0;
    char *str = NULL;
    char *cellstr[chart->size][chart->size];

    for (i=0; i < chart->size; i++){
        int len = strlen(chart->input[i]);
        if (len > max_len) max_len = len;
    }

    for (i=0; i < chart->size; i++){
        for (j=0; j < chart->size - i; j++) {
            struct chart_node *node = chart->node[i][j];
            if (node) {
                size_t  len;
                char    tmpnum[5];
                int     child_count = 0;
                struct backlink *b = node->back;
                while(b) {
                    b = b->next;
                    child_count++;
                }
                snprintf(tmpnum, 5, ":%d", child_count);
                str = str_astrcat(&str, node->cat->str);
                str = str_astrcat(NULL, tmpnum);
                node = node->next;
                while(node) {
                    b = node->back;
                    child_count = 0;
                    while(b) {
                        b = b->next;
                        child_count++;
                    }
                    snprintf(tmpnum, 5, ":%d", child_count);
                    str = str_astrcat(NULL, ",");
                    str = str_astrcat(NULL, node->cat->str);
                    str = str_astrcat(NULL, tmpnum);
                    node = node->next;
                }
                len = strlen(str);
                if(len > max_len) max_len = len;
                cellstr[i][j] = str;
            } else {
                cellstr[i][j] = NULL;
            }
        }
    }

    fprintf(fp, "------ Chart: \n");
    for(i=0; i < chart->size; i++){
        fprintf(fp, "%-*s |", max_len, chart->input[i]);
    }
    fprintf(fp, "\n");

    for(i=0; i < chart->size; i++){
        for(j=0; j < chart->size - i; j++) {
            char *s = (cellstr[i][j]) ? cellstr[i][j] : "";
            fprintf(fp, "%-*s |", max_len, s);
            if(cellstr[i][j]) free(cellstr[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "------ End of chart\n");
}

struct parse_tree *
parse_tree_new(struct chart_node *cnode, struct backlink *b)
{
    struct parse_tree *t;

    t = malloc(sizeof (*t));
    t->node = cnode;
    t->b = b;
    t->L = NULL;
    t->R = NULL;
    t->expanded = 0;
    return t;
};

struct parse_tree *
parse_tree_node_dup(struct parse_tree *src)
{
    struct parse_tree *dst = parse_tree_new(src->node, src->b);
    dst->L = src->L;
    dst->R = src->R;
    dst->expanded = src->expanded;
    return dst;
}

struct parse_tree *
parse_tree_dup_r (struct parse_tree *src, 
                  struct parse_tree *root,
                  struct parse_tree *src_node,
                  struct parse_tree *dst_node,
                  struct stack      *st)
{
    if(src == NULL) {
        return NULL;
    }

    struct parse_tree *t = NULL;

    if (src == src_node) {
        t = dst_node;
    } else {
        t = parse_tree_node_dup(src);
    }

    if (t->b == NULL) {
        return t;
    }
    if (!src->L || !src->R) { // not a leaf node, incomplete/active node 
        stack_push(st, t);    // push to the stack.
        stack_push(st, root);
        PDEBUG(100, "V1 r:%p<%s:%p,b:%p,L:%p,R:%p>, "
                      "t:%p<%s:%p,b:%p,L:%p,R:%p>\n",
                     root, root->node->cat->str, root->node, root->b, root->L, root->R,
                     t, t->node->cat->str, t->node, t->b, t->L, t->R);
    }

    t->L = parse_tree_dup_r(src->L, root, src_node, dst_node, st);
    t->R = parse_tree_dup_r(src->R, root, src_node, dst_node, st);
    return t;
}

struct parse_tree *
parse_tree_dup(struct parse_tree *src, 
               struct parse_tree *src_node,
               struct parse_tree *dst_node,
               struct stack   *st)
{
    struct parse_tree *new = parse_tree_node_dup(src);


    new->L = parse_tree_dup_r(src->L, new, src_node, dst_node, st);
    new->R = parse_tree_dup_r(src->R, new, src_node, dst_node, st);

    return new;
}

struct parse_list *
parse_list_add(struct parse_list **l, struct parse_tree *t)
{
    struct parse_list *new = malloc (sizeof (*new));

    new->t = t;
    new->parse_num = 0;
    new->next = *l;
    *l = new;
    if (new->next) {
        new->parse_num = new->next->parse_num + 1;
    }
    return new;
}

struct parse_list *
get_parselist_full(struct chart *chart)
{
    struct parse_list   *ret = NULL;
    struct stack        *st = stack_init();

    if (chart == NULL) {
       fprintf(stderr,"Chart is NULL\n");   
       return NULL;
    }

    if (NULL == chart->node[chart->size-1][0]) {
       fprintf(stderr,"No full parses\n");   
       return NULL;
    }

    struct chart_node *cnode = chart->node[chart->size-1][0];
    struct backlink *b = cnode->back;
    struct parse_tree *r = NULL;

    while(b) {
        r = parse_tree_new(cnode, b);
        parse_list_add(&ret, r);
        stack_push(st, r);
        stack_push(st, r);
        r->expanded = 1;
        PDEBUG(100, "V3 r:%p<%s:%p,b:%p,L:%p,R:%p>, "
                      "t:%p<%s:%p,b:%p,L:%p,R:%p>\n",
                     r, r->node->cat->str, r->node, r->b, r->L, r->R,
                     r, r->node->cat->str, r->node, r->b, r->L, r->R);
        b = b->next;
    }
    
    while (st->next) {
        struct parse_tree *troot = stack_pop(st);
        struct parse_tree *tnode = stack_pop(st);
        struct parse_tree *t = NULL;
        PDEBUG(100, "^ r:%p<%s:%p,b:%p,L:%p,R:%p,%c>, t:%p<%s:%p,b:%p,L:%p,R:%p,%c>\n",
                     troot, troot->node->cat->str, troot->node, troot->b, troot->L, troot->R, (troot->expanded) ? '+' : '-',
                     tnode, tnode->node->cat->str, tnode->node, tnode->b, tnode->L, tnode->R, (tnode->expanded) ? '+' : '-');

        if (tnode->b) {
            b = tnode->b;
            if (b->next && !tnode->expanded) {
                tnode->expanded = 1;
                while(b->next) {
                    struct parse_tree *ntnode;
                    ntnode = parse_tree_new(tnode->node, b->next);
                    ntnode->expanded = 1; // FIXME: this should be redundant
                    r = parse_tree_dup(troot, tnode, ntnode, st);
                    PDEBUG(100, "d %p -> %p\n", troot, r);
                    parse_list_add(&ret, r);
                    b = b->next;
                }
            }

            b = tnode->b;
            t = parse_tree_new(b->R, b->R->back);
            tnode->R = t;
            if (t->b) { // push only if the node is not a leaf node
                stack_push(st, t);
                stack_push(st, troot);
                PDEBUG(100, "V4r r:%p<%s:%p,b:%p,L:%p,R:%p>, "
                              "t:%p<%s:%p,b:%p,L:%p,R:%p>\n",
                             troot, troot->node->cat->str, troot->node, troot->b, troot->L, troot->R,
                             t, t->node->cat->str, t->node, t->b, t->L, t->R);
            }

            t = parse_tree_new(b->L, b->L->back);
            tnode->L = t;
            if (t->b) { // push only if the node is not a leaf node
                stack_push(st, t);
                stack_push(st, troot);
                PDEBUG(100, "V4l r:%p<%s:%p,b:%p,L:%p,R:%p>, "
                              "t:%p<%s:%p,b:%p,L:%p,R:%p>\n",
                             troot, troot->node->cat->str, troot->node, troot->b, troot->L, troot->R,
                             t, t->node->cat->str, t->node, t->b, t->L, t->R);
            }

        PDEBUG(100, "<%p> %s<%p> -> %s<%p> %s<%p>\n", troot, tnode->node->cat->str, tnode,
                    (tnode->L) ? tnode->L->node->cat->str : "(nil)", tnode->L,
                    (tnode->R) ? tnode->R->node->cat->str : "(nil)", tnode->R);

        }


    }

    stack_free(st, 0);
    return ret;
}

void
write_parse(FILE *fp, struct parse_tree *t, struct chart *c)
{

    if(t->L == NULL) {
        char *tmps = str_astrcat(&tmps, c->input[t->node->sp_start]);
        int  i;
        for (i=1; i < t->node->sp_len; i++) {
            tmps = str_astrcat(NULL, c->input[t->node->sp_start+i]);
        }

        fprintf(fp, "%s -> %s\n", t->node->cat->str, tmps);

        free(tmps);
        return;
    }

    fprintf(fp, "%s -> %s %s\n", t->node->cat->str, 
                                 t->L->node->cat->str,
                                 t->R->node->cat->str);
    
    write_parse(fp, t->L, c);
    write_parse(fp, t->R, c);
}

void 
write_parses(FILE *fp, struct chart *chart)
{
    if (chart == NULL) {
       fprintf(stderr,"Chart is NULL\n");   
       return;
    }

    if(!chart->node[chart->size-1][0]) {
       fprintf(stderr,"No full parses\n");   
       return;
    }

    struct parse_list *parses = get_parselist_full(chart);

    struct parse_list *tmp = parses;
    printf("%zd parses.\n", tmp->parse_num + 1);
    while (tmp) {
        printf("--------- %zd ----------\n", tmp->parse_num);
        write_parse(fp, tmp->t, chart);
        tmp = tmp->next;
    }
}

