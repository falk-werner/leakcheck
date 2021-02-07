#include "leakcheck_simple/leakcheck_simple.h"

void * malloc(size_t size)
{
    return leakcheck_simple_malloc(size);
}

void * calloc(size_t count, size_t size)
{
    return leakcheck_simple_calloc(count, size);
}

void * realloc(void * ptr, size_t size)
{
    return leakcheck_simple_realloc(ptr, size);
}

void free(void * ptr)
{
    leakcheck_simple_free(ptr);
}