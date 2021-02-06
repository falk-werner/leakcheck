#include "leakcheck/leakcheck.h"

void * malloc(size_t size)
{
    return leakcheck_malloc(size);
}

void * calloc(size_t count, size_t size)
{
    return leakcheck_calloc(count, size);
}

void * realloc(void * ptr, size_t size)
{
    return leakcheck_realloc(ptr, size);
}

void free(void * ptr)
{
    leakcheck_free(ptr);
}