#include <stdio.h>
#include <stdlib.h>
#include "trie.h"

struct trie *trie_init(size_t size)
{
    struct trie *t = malloc(sizeof *t);

    t->size = size;
    t->types = 0;
    t->root = malloc(sizeof *t->root);
    t->root->children = NULL;
    t->root->count = 0;
    t->root->count_final = 0;
    return t;
}

void trie_free_recursive(struct trie_node *node, size_t size)
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

void trie_insert(struct trie *trie, segunit_t *seq)
{
    trie_insert_span(trie, seq, 0, 0);
}

void trie_insert_span(struct trie *trie, segunit_t *seq,
        size_t start, size_t length)
{
    struct trie_node *node = trie->root;
    size_t i;

    node->count += 1;
    for (i = 0; (length) ? (i < length) : true; i++) {
        segunit_t *val = seq + start + i;
        if (*val == 0)
            break;
        if (node->children == NULL) {
            node->children = calloc(trie->size, sizeof *node->children);
        }
        if (node->children[*val] == NULL) {
            node->children[*val] = calloc(1, sizeof *node->children[*val]);
        }
        node = node->children[*val];
        node->count += 1;
    }
    if (node->count_final == 0)
        trie->types += 1;
    node->count_final += 1;
}

struct trie_node *trie_lookup(struct trie *trie, segunit_t *seq)
{
    return trie_lookup_span(trie, seq, 0, 0);
}

struct trie_node *trie_lookup_span(struct trie *trie, segunit_t *seq,
        size_t start, size_t length)
{
    struct trie_node *node = trie->root;
    size_t i;

    for (i = 0; (length) ? (i < length) : true; i++) {
        segunit_t *val = seq + start + i;
        if (*val == 0)
            break;
        if (node->children == NULL) {
                return NULL;
        } else if (node->children[*val] == NULL) {
                return NULL;
        } else {
            node = node->children[*val];
        }
    }
    return node;
}


double trie_relfreq_word(struct trie *trie, segunit_t *seq)
{
    return trie_relfreq_span(trie, seq, 0, 0);
}

double trie_relfreq_span(struct trie *trie, segunit_t *seq, 
        size_t start, size_t length)
{
    struct trie_node *n = trie_lookup_span(trie, seq, start, length);

    if (n == NULL) 
        return 0.0;
    else 
        return (double) n->count_final / (double) trie->root->count;
}


#ifdef TRIE_TEST

segunit_t seq[10][6] = {{1, 2, 3, 4, 0},    // 0
                        {2, 3, 4, 5, 0},    // 1
                        {2, 1, 4, 0},       // 2
                        {1, 2, 1, 0},       // 3
                        {0},                // 4
                        {1, 2, 0},          // 5
                        {2, 1, 0},          // 6
                        {2, 2, 0},          // 7
                        {2, 0},             // 8
                        {3, 0}              // 9
                   };

int main()
{
    struct trie *t = trie_init(10);
    struct trie_node *tn;

    trie_insert(t, seq[0]);
    trie_insert(t, seq[0]);
    trie_insert(t, seq[1]);
    trie_insert(t, seq[2]);
    trie_insert(t, seq[3]);
    trie_insert(t, seq[4]);
    trie_insert(t, seq[4]);

    tn = trie_lookup(t, seq[0]);
    if (tn != NULL) {
        printf("0: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[3]);
    if (tn != NULL) {
        printf("3: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[4]);
    if (tn != NULL) {
        printf("4: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[5]);
    if (tn != NULL) {
        printf("5: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[6]);
    if (tn != NULL) {
        printf("6: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[7]);
    if (tn != NULL) {
        printf("7: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[8]);
    if (tn != NULL) {
        printf("8: %zu/%zu\n", tn->count_final, tn->count);
    }
    tn = trie_lookup(t, seq[9]);
    if (tn != NULL) {
        printf("9: %zu/%zu\n", tn->count_final, tn->count);
    }

    trie_free(t);
    return 0;
}

#endif // ifdef TRIE_TEST
