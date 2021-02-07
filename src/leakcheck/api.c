#include "leakcheck/leakcheck.h"

void * malloc(size_t size)
{
    void * caller =  __builtin_extract_return_addr (__builtin_return_address (0));
    return leakcheck_malloc(size, caller);
}

void * calloc(size_t count, size_t size)
{
    void * caller =  __builtin_extract_return_addr (__builtin_return_address (0));
    return leakcheck_calloc(count, size, caller);
}

void * realloc(void * ptr, size_t size)
{
    void * caller =  __builtin_extract_return_addr (__builtin_return_address (0));
    return leakcheck_realloc(ptr, size, caller);
}

void free(void * ptr)
{
    void * caller =  __builtin_extract_return_addr (__builtin_return_address (0));
    leakcheck_free(ptr, caller);
}