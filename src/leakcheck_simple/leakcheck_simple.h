#ifndef LEAKCHECK_SIMPLE_H
#define LEAKCHECK_SIMPLE_H

#ifndef __cplusplus
#include <stddef.h>
#else
#include <cstddef>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern void * leakcheck_simple_malloc(size_t size);

extern void * leakcheck_simple_calloc(size_t count, size_t size);

extern void * leakcheck_simple_realloc(void * ptr, size_t size);

extern void leakcheck_simple_free(void * ptr);


#ifdef __cplusplus
}
#endif

#endif
