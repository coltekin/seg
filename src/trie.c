#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "input.h"

extern struct input *inp_dbg; // REMOVE ME

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
//fprintf(stderr, "--- len %zu ... \n", len);
    for (i = 0; i < len; i++) {
        segunit_t edge = seq[i];
//fprintf(stderr, "\tinserting %s(%hu) ... ", inp_dbg->sigma[edge].str, edge);
        if (node->children == NULL) {
//fprintf(stderr, "first edge ... ");
            node->children = calloc(trie->size, sizeof *node->children);
        }
        if (node->children[edge] == NULL) {
//fprintf(stderr, "new edge ... ");
            node->children[edge] = calloc(1, sizeof *node->children[edge]);
        }
        node = node->children[edge];
        node->count += 1;
//fprintf(stderr, "node count: %zu\n", node->count);
    }
    if (node->count_final == 0)
        trie->types += 1;
    node->count_final += 1;
//fprintf(stderr, "\tnode final count: %zu\n", node->count_final);
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
