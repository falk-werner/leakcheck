#define _GNU_SOURCE

#include "leakcheck_simple/leakcheck_simple.h"

#include <unistd.h>
#include <dlfcn.h>
#include <strings.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

typedef void * malloc_fn(size_t);
typedef void * realloc_fn(void *, size_t);
typedef void free_fn(void *);

static void * early_malloc(size_t);

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t alloc_count = 0;
static size_t alloc_count_total = 0;

static malloc_fn * real_malloc = &early_malloc;
static realloc_fn * real_realloc = NULL;
static free_fn * real_free = NULL;


static void * get_symbol(char const * symbol_name)
{
    void * symbol = dlsym(RTLD_NEXT, symbol_name);
    if (NULL == symbol)
    {
        fprintf(stderr, "error: failed to load symbol: %s\n", symbol_name);
        exit(EXIT_FAILURE);
    }

    return symbol;
}

static void get_commandline(char * buffer, size_t buffer_size)
{
    if (0 == buffer_size) { return; }

    FILE * file = fopen("/proc/self/cmdline", "rb");
    if (NULL != file)
    {
        size_t length = fread(buffer, 1 , buffer_size - 1, file);
        for (size_t i = 0; i < length; i++)
        {
            if ('\0' == buffer[i])
            {
                buffer[i] = ' ';
            }
        }
        buffer[length] = '\0';
        fclose(file);
    }
}

static void cleanup() __attribute__((constructor));
static void cleanup(void)
{
    char cmdline[80];
    get_commandline(cmdline, 80);
    char buffer[1024];
    int len = snprintf(buffer, 1024, 
        "\n"
        "=== Leakcheck (simple, %s) ===\n"
        "blocks in use at exit : %zu\n"
        "total allocated blocks: %zu\n",
    cmdline,
    alloc_count, 
    alloc_count_total);
    write(STDERR_FILENO, buffer, len);
}

enum state 
{
    uninitialized,
    initializing,
    initialized
};

static enum state g_state = uninitialized;

static void init(void)
{
    if (uninitialized == g_state)
    {
        g_state = initializing;
        real_malloc = get_symbol("malloc");
        real_free = get_symbol("free");
        real_realloc = get_symbol("realloc");

        //atexit(cleanup);
        g_state = initialized;

    }
}


#define EARLY_HEAP_SIZE 1024
char early_heap[EARLY_HEAP_SIZE];
size_t early_heap_offset = 0;

void * early_malloc(size_t size)
{
    void * result = NULL;

    if ((0 < size) && ((early_heap_offset + size) < EARLY_HEAP_SIZE))
    {
        result = &early_heap[early_heap_offset];
        early_heap_offset += size;
    }

    return result;
}

void * leakcheck_simple_malloc(size_t size)
{
    init();

    void * ptr = real_malloc(size);
    if (NULL != ptr)
    {
        pthread_mutex_lock(&stats_mutex);
        alloc_count++;
        alloc_count_total++;
        pthread_mutex_unlock(&stats_mutex);
    }

    return ptr;
}

void * leakcheck_simple_calloc(size_t count, size_t size)
{
    init();
    if ((0 == count) || (0 == size)) { return NULL; }

    void * ptr = leakcheck_simple_malloc(count * size);
    if (NULL != ptr)
    {
        bzero(ptr, count * size);
    }

    return ptr;
}

void * leakcheck_simple_realloc(void *ptr, size_t size)
{
    init();
    if ((NULL == ptr) && (0 == size))
    {
        return NULL;
    }
    else if (NULL == ptr)
    {
        return leakcheck_simple_malloc(size);
    }
    else if (0 == size)
    {  
        leakcheck_simple_free(ptr);
        return NULL;
    }
    else
    {
        return real_realloc(ptr, size);
    }
    
}

void leakcheck_simple_free(void * ptr)
{
    if (NULL == ptr) { return; }

    void * heap_start = (void*) early_heap;
    void * heap_end = (void*) (&early_heap[EARLY_HEAP_SIZE]);
    if ((heap_start <= ptr) && (ptr < heap_end))
    {
        // do nothing
    }
    else
    {
        real_free(ptr);
    }

    pthread_mutex_lock(&stats_mutex);
    alloc_count--;
    pthread_mutex_unlock(&stats_mutex);
}
