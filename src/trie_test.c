#include <stdio.h>
#include "trie.h"

segunit_t inseq[6][3] = {{1},
                       {1, 1},
                       {1, 2},
                       {2, 3},
                       {2, 1, 1},
                       {3, 2}
};

size_t inseqlen[] = {1, 2, 2, 2, 3, 2};

int main()
{
    struct trie *t = trie_init(4);
    struct trie_node *tn;
    struct trie_iter *ti;
    segunit_t *seq;
    size_t seqlen;
    size_t i;

    for (i = 0; i < 6; i++) {
        trie_insert(t, inseq[i], inseqlen[i]);
    }

    ti = trie_iter_init(t);
    while((tn = trie_iter_next(ti, &seq, &seqlen))) {
        for (i = 0; i < seqlen; i++) {
            printf("%hu ", seq[i]);
        }
        if (tn->count_final) printf("*");
        printf("\n");
    }

    trie_iter_free(ti);
    trie_free(t);
    return 0;
}
