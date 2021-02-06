#ifndef LEAKCHECK_H
#define LEAKCHECK_H

#ifndef __cplusplus
#include <stddef.h>
#else
#include <cstddef>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern void * leakcheck_malloc(size_t size);

extern void * leakcheck_calloc(size_t count, size_t size);

extern void * leakcheck_realloc(void * ptr, size_t size);

extern void leakcheck_free(void * ptr);


#ifdef __cplusplus
}
#endif

#endif
