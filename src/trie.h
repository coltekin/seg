#ifndef _TRIE_H
#define _TRIE_H 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "input.h"

/* trie_node holds data for a node in the trie.
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
    size_t  size;   // size of the alphabet
    size_t  types;  // the number of types inserted, for tokens use root->count
};

/* trie_init() - initialize and return an empty trie
 * @size         is the size of the alphabet. The members of the
 *               alphabet are integers (segunit_t for this
 *               implementation). The null value is used 
 *               for marking the end of the sequences, is not counted
 *               for the size, i.e., @size=4 means values 1,2,3,4 are
 *               valid in a sequence.
 *
 */
struct trie *trie_init(size_t size);

/* trie_free() - destroy the contents of the @trie, and free the
 *               memory allocated for it.
 * @trie         trie to free.
 */
void trie_free(struct trie *trie);

/* trie_insert() - insert a sequence into the @trie
 * @trie           the target trie.
 * @seq            the null-terminated sequence to insert. 
 */
void trie_insert(struct trie *trie, segunit_t *seq);

/* trie_insert() - insert a span of the given sequence into the @trie
 * @trie           the target trie.
 * @seq            the null-terminated sequence to insert. 
 * @start          the start position of the span
 * @length         the length of the span, 0 is interpreted as untill
 *                 the end of the span
 */
void trie_insert_span(struct trie *trie, segunit_t *seq, 
        size_t start, size_t length);

/* trie_lookup - lookup given sequence in the trie and return final node
 * @trie           the target trie.
 * @seq            the null-terminated sequence to lookup. 
 */
struct trie_node *trie_lookup(struct trie *trie, segunit_t *seq);

/* trie_lookup_span - lookup a span of the given sequence and return final node
 * @trie              the target trie.
 * @seq               the null-terminated sequence to lookup. 
 * @start             the start position of the span
 * @length            the length of the span, 0 is interpreted as untill
 *                    the end of the span
 */
struct trie_node *trie_lookup_span(struct trie *trie, segunit_t *seq,
        size_t start, size_t length);

/* trie_relfreq_word() - retrn the relative freq. of a word.
 * @trie                 the trie
 * @seq                  the 0-termintated index sequence corresponding 
 *                       to the word
 */
double trie_relfreq_word(struct trie *trie, segunit_t *seq);

/* trie_relfreq_word() - retrn the relative freq. of the given span
 *                       (as aw word)
 * @trie                 the trie
 * @seq                  the complete 0-termintated index sequence 
 * @start                the start position of the span
 * @length               the length of the span, 0 is interpreted as untill
 *                       the end of the span
 */
double trie_relfreq_span(struct trie *trie, segunit_t *seq, 
        size_t start, size_t length);

#endif // _TRIE_H

