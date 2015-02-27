#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "trie.h"
#include "input.h"
#include "xalloc.h"

extern struct input *inp_dbg; // REMOVE ME
size_t idx_dbg; // REMOVE ME

struct trie *trie_init(size_t size)
{
    struct trie *t = malloc(sizeof *t);

    t->size = size;
    t->types = 0;
    t->max_depth = 0;
    t->root = malloc(sizeof *t->root);
    t->root->children = NULL;
    t->root->count = 0;
    t->root->count_final = 0;
    return t;
}

static void trie_free_recursive(struct trie_node *node, size_t size)
{
    if (node->children != NULL) {
        size_t i;
        for (i = 0; i < size; i++) {
            if (node->children[i] != NULL) 
                trie_free_recursive(node->children[i], size);
        }
        free(node->children);
    }
    free(node);
}

void trie_free(struct trie *trie)
{
    trie_free_recursive(trie->root, trie->size);
    free(trie);
}

void trie_insert(struct trie *trie, segunit_t *seq, size_t len)
{
    struct trie_node *node = trie->root;
    size_t i;

    node->count += 1;
    for (i = 0; i < len; i++) {
        segunit_t edge = seq[i];
        if (node->children == NULL) {
            node->children = xcalloc(trie->size, sizeof *node->children);
        }
        if (node->children[edge] == NULL) {
            node->children[edge] = xcalloc(1, sizeof *node->children[edge]);
        }
        node = node->children[edge];
        node->count += 1;
    }
    if (node->count_final == 0) {
        trie->types += 1;
    }
    node->count_final += 1;

    if (len > trie->max_depth) trie->max_depth = len;
}

struct trie_node *trie_lookup(struct trie *trie, segunit_t *seq, size_t len)
{
    struct trie_node *node = trie->root;
    size_t i;

    for (i = 0; i < len; i++) {
        segunit_t edge = seq[i];
        if (node->children == NULL) {
                return NULL;
        } else if (node->children[edge] == NULL) {
                return NULL;
        } else {
            node = node->children[edge];
        }
    }
    return node;
}

size_t trie_freq(struct trie *trie, segunit_t *seq, size_t len)
{
    struct trie_node *n = trie_lookup(trie, seq, len);
    return n->count_final;
}

size_t trie_prefix_freq(struct trie *trie, segunit_t *seq, size_t len)
{
    struct trie_node *n = trie_lookup(trie, seq, len);
    return n->count;
}

double trie_relfreq(struct trie *trie, segunit_t *seq, size_t len)
{
    struct trie_node *n = trie_lookup(trie, seq, len);

    if (n == NULL)  {
        return 0.0;
    } else  {
        return (double) n->count_final / (double) trie->root->count;
    }
}

struct trie_iter_data {
    struct trie_node *node;
    size_t depth;
    segunit_t label;
};

struct trie_iter *trie_iter_init(struct trie *t)
{
    struct trie_iter *ti = xmalloc(sizeof *ti);
    struct trie_iter_data *d = xmalloc(sizeof *d);
    d->node = t->root;
    d->depth = 0;
    d->label = 0;
    ti->trie = t;
    ti->seq = xmalloc(t->max_depth * sizeof *ti->seq);
    ti->stack = stack_init();
    stack_push(ti->stack, d);
    return ti;
}

void trie_iter_free(struct trie_iter *ti)
{
    stack_free(ti->stack, true);
    free(ti->seq);
    free(ti);
}


struct trie_node *trie_iter_next(struct trie_iter *ti, segunit_t **seq, size_t *len)
{
    struct trie_iter_data *d;
    struct trie_node *node;
    size_t i;

    if (stack_is_empty(ti->stack)) {
        len = 0;
        *seq = NULL;
        return NULL;
    }

    d = stack_pop(ti->stack);
    node = d->node;
    for (i = 0; node->children != NULL && i < ti->trie->size; i++) {
        if (node->children[i] != NULL) {
            struct trie_iter_data *child = malloc(sizeof *d);
            child->node = node->children[i];
            child->depth = d->depth + 1;
            child->label = i;
            stack_push(ti->stack, child);
        }
    }

    if (d->depth > 0) {
        ti->seq[d->depth - 1] = d->label;
    }
    *len = d->depth;
    *seq = ti->seq;
    free(d);
    return node;
}

