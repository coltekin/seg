#ifndef _TRIE_H
#define _TRIE_H 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "input.h"
#include "stack.h"

/**
 * trie_node holds data for a node in the trie.
 * the children are stored as an array (of size trie.size)
 * allocated at first time a child is inserted.
 */

struct trie_node {
    struct trie_node **children; // an array of children
    size_t           count; // number of times this prefix was inserted
    size_t           count_final; // number of times this prefix was final
};

struct trie {
    struct trie_node *root; // pointer to the root
    size_t size;   // size of the alphabet
    size_t types;  // the number of types inserted, for tokens use root->count
    size_t max_depth; // maximum depth of the trie
};

/**
 * trie_iter - used for iterating over a tree
 */
struct trie_iter {
    struct trie *trie;
    struct stack *stack;
    segunit_t *seq;
};

/**
 * trie_init() - initialize and return an empty trie
 * @size:        is the size of the alphabet. The members of the
 *               alphabet are integers (segunit_t for this
 *               implementation). The null value is used 
 *               for marking the end of the sequences, is not counted
 *               for the size, i.e., @size=4 means values 1,2,3,4 are
 *               valid in a sequence.
 *
 */
struct trie *trie_init(size_t size);

/**
 * trie_free() - destroy the contents of the @trie, and free the
 *               memory allocated for it.
 * @trie:        trie to free.
 */
void trie_free(struct trie *trie);

/**
 * trie_insert() - insert a sequence into the @trie
 * @trie:          the target trie.
 * @seq:           pointer to the beginning of the sequence
 * @len:           length of the sequence
 */
void trie_insert(struct trie *trie, segunit_t *seq, size_t len);

/**
 * trie_lookup - lookup given sequence in the trie and return the final node
 * @trie:        the target trie.
 * @seq:         the sequence to lookup. 
 * @len:         length of the sequence
 */
struct trie_node *trie_lookup(struct trie *trie, segunit_t *seq, size_t len);

/**
 * trie_freq() - retrn the relative freq. of a word.
 * @trie         the trie
 * @seq:         the sequence
 * @len:         length of the sequence
 *
 */
size_t trie_freq(struct trie *trie, segunit_t *seq, size_t len);

/**
 * trie_prefix_fres() - retrn the relative freq. of a word.
 * @trie:               the trie
 * @seq:                the sequence
 * @len:                length of the sequence
 *
 * This returns counts of prefixes. For example 'abc' and 'abcd' is
 * inserted into the trie, the sequence  'ab' will return 2, unlike
 * trie_freq() which would return 0 for 'ab' and 1 for full sequences,
 * 'abc' and 'abcd', only.
 */
size_t trie_prefix_freq(struct trie *trie, segunit_t *seq, size_t len);

/**
 * trie_relfreq() - retrn the relative freq. of a word.
 * @trie:           the trie
 * @seq:            the 0-termintated index sequence corresponding 
 * @len:            length of the sequence
 */
double trie_relfreq(struct trie *trie, segunit_t *seq, size_t len);

/**
 * triel_iter_init() - initalize a trie_init structure
 * @t:                 the trie
 */
struct trie_iter *trie_iter_init(struct trie *t);

/**
 * triel_iter_free() - free the resources allocated by a trie_iter
 * @ti:                the structure to destroy
 */
void trie_iter_free(struct trie_iter *ti);

/**
 * triel_iter_iter() - return the next prefix in the trie 
 * @ti:                tie handle for the iteration
 * @seq:               pointer to the edge label sequence 
 * @len:               pointer to the length of the sequence
 *
 * This function returns each node in the trie (with a depth-first
 * traversal), along with the sequences of the edge leables that leads
 * to the node. The pointer @seq returned may be modified with the
 * next call to this function. The user should copy it if necessary.
 * The return value is either a pointer to the trie node, or NULL if
 * all nodes are returned in the earlier calls.
 *
 * The trie should not be changed during the traversal.
 */
struct trie_node *trie_iter_next(struct trie_iter *ti, segunit_t **seq, size_t *len);

#endif // _TRIE_H

