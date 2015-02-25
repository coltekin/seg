#include <stdlib.h>
#include <stdio.h>

void *xmalloc (size_t size)
{
    void *new = malloc (size);
    if (new == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(-1);
    }
    return new;
}

void *xrealloc (void *p, size_t newsize)
{
    void *new = realloc (p, newsize);
    if (new == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(-1);
    }
    return new;
}

void *xcalloc (size_t num, size_t size)
{
    void *new = calloc(num, size);
    if (new == NULL) {
        fprintf(stderr, "Not enough memory\n");
        exit(-1);
    }
    return new;
}
