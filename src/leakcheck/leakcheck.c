#define _GNU_SOURCE

#include "leakcheck/leakcheck.h"

#include <unistd.h>
#include <dlfcn.h>
#include <strings.h>
#include <execinfo.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

#define MAGIC 0xAF0015BAD1DEA542
#define STACK_TRACE_SIZE 5

typedef void * malloc_fn(size_t);
typedef void free_fn(void *);
typedef int backtrace_fn(void **, int);

struct block_info
{
    uint64_t magic;    
    size_t size;

    void * stack_trace[STACK_TRACE_SIZE];
    int stack_trace_items;

    struct block_info * next;
    struct block_info * prev;
};

static void * early_malloc(size_t);
static int early_backtrace(void * * buffer, int);

static size_t ignored_count = 0;
static size_t ignored_size = 0;
static size_t alloc_count = 0;
static size_t alloc_size = 0;
static size_t alloc_size_total = 0;
static size_t malloc_count = 0;
static size_t calloc_count = 0;

static malloc_fn * real_malloc = &early_malloc;
static free_fn * real_free = NULL;
static backtrace_fn * real_backtrace = &early_backtrace;

static struct block_info blocks = 
{
    .magic = 0xAF0015BAD1DEA542,
    .size = 0,
    .next = &blocks,
    .prev = &blocks
};
pthread_mutex_t blocks_mutex = PTHREAD_MUTEX_INITIALIZER;

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

static void cleanup(void)
{
    char buffer[1024];
    int len = snprintf(buffer, 1024, 
        "\n"
        "=== Leakcheck ===\n"
        "In use at exit:\n"
        "\tblocks : %zu\n"
        "\tignored: %zu\n"
        "\tsize   : %zu\n"
        "total allocated blocks: %zu\n"
        "total allocated size  : %zu\n",
     alloc_count - ignored_count, ignored_count, alloc_size - ignored_size, 
     malloc_count + calloc_count - ignored_count,
     alloc_size_total);
    write(STDERR_FILENO, buffer, len);

    if (0 < (alloc_count - ignored_count))
    {
        len = snprintf(buffer, 1024, "\n=== Available Blocks ===\n");
        write(STDERR_FILENO, buffer, len);

        struct block_info * current = blocks.next;
        while (current != &blocks)
        {
            if (0 < current->stack_trace_items)
            {
                len = snprintf(buffer, 1024, "\nblock size: %zu\n", current->size);
                write(STDERR_FILENO, buffer, len);
                if (0 < current->stack_trace_items)
                {
                    backtrace_symbols_fd(current->stack_trace, current->stack_trace_items, STDERR_FILENO);
                }
            }
            current = current->next;
        }
    }
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

        void * dummy;
        backtrace(&dummy, 1);
        real_backtrace = get_symbol("backtrace");

        atexit(cleanup);
        g_state = initialized;

    }
}

void add_block(struct block_info * new_block)
{
    pthread_mutex_lock(&blocks_mutex);

    struct block_info * last_block = blocks.prev;
    new_block->prev = last_block;
    new_block->next = &blocks;
    last_block->next = new_block;
    blocks.prev = new_block;

    pthread_mutex_unlock(&blocks_mutex);
}

void remove_block(struct block_info * block_to_remove)
{
    pthread_mutex_lock(&blocks_mutex);

    struct block_info * next = block_to_remove->next;
    struct block_info * prev = block_to_remove->prev;

    next->prev = prev;
    prev->next = next;

    pthread_mutex_unlock(&blocks_mutex);
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

static int early_backtrace(void * * buffer, int size)
{
    (void) buffer;
    (void) size;
    return 0;
}

void * leakcheck_malloc(size_t size)
{
    init();

    if (0 == size) { return NULL; }

    void * result = NULL;
    void * ptr = real_malloc(sizeof(struct block_info) + size);
    if (NULL != ptr)
    {
        struct block_info * info = ptr;
        info->magic = MAGIC;
        info->size = size;
        info->stack_trace_items = real_backtrace(info->stack_trace, STACK_TRACE_SIZE);
        add_block(info);
        result = ( ((char*) ptr) + sizeof(struct block_info) );

        if (0 == info->stack_trace_items)
        {
            ignored_count++;
            ignored_size += size;
        }

        alloc_count++;
        alloc_size += size;
        alloc_size_total += size;
        malloc_count++;
    }

    return result;
}

void * leakcheck_calloc(size_t count, size_t size)
{
    init();
    if ((0 == count) || (0 == size)) { return NULL; }

    void * result = NULL;
    void * ptr = real_malloc(sizeof(struct block_info) + (count * size));
    if (NULL != ptr)
    {
        struct block_info * info = ptr;
        info->magic = MAGIC;
        info->size = (count * size);
        info->stack_trace_items = real_backtrace(info->stack_trace, STACK_TRACE_SIZE);
        add_block(info);
        result = ( ((char*) ptr) + sizeof(struct block_info) );
        bzero(result, info->size);

        if (0 == info->stack_trace_items)
        {
            ignored_count++;
            ignored_size += (count * size);
        }

        alloc_count++;
        alloc_size += (count * size);
        alloc_size_total += (count * size);
        calloc_count++;
    }

    return result;
}

void * leakcheck_realloc(void *ptr, size_t size)
{
    init();
    if ((NULL == ptr) && (0 == size))
    {
        return NULL;
    }
    else if (NULL == ptr)
    {
        return malloc(size);
    }
    else if (0 == size)
    {  
        free(ptr);
        return NULL;
    }
    else
    {
        struct block_info * info = (struct block_info*) ( ((char*) ptr) - sizeof(struct block_info) );
        size_t const old_size = info->size;
        void * result = ptr;

        if (old_size < size)
        {
            result = malloc(size);
            if (NULL != result)
            {
                memcpy(result, ptr, old_size);
                free(ptr);
            }
        }

        return result;
    }
    
}

void leakcheck_free(void * ptr)
{
    if (NULL == ptr) { return; }

    struct block_info * info = (struct block_info*) ( ((char*) ptr) - sizeof(struct block_info) );
    if (MAGIC == info->magic)
    {
        alloc_count--;
        alloc_size -= info->size;
        remove_block(info);

        void * heap_start = (void*) early_heap;
        void * heap_end = (void*) (&early_heap[EARLY_HEAP_SIZE]);
        if ((heap_start <= ptr) && (ptr < heap_end))
        {
            // do nothing
        }
        else
        {
            real_free(info);
        }
    }
    else
    {
        fprintf(stderr, "error: bad free %p\n", ptr);
        void * buffer[5];
        int len = backtrace(buffer, 5);
        backtrace_symbols_fd(buffer, len, STDERR_FILENO);
    }
}
