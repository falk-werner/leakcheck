#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

static void print_mallinfo(void)
{
    struct mallinfo info = mallinfo();

    printf("=== mallinfo === \n");
    printf("Total non-mmapped bytes (arena):       %d\n", info.arena);
    printf("# of free chunks (ordblks):            %d\n", info.ordblks);
    printf("# of free fastbin blocks (smblks):     %d\n", info.smblks);
    printf("# of mapped regions (hblks):           %d\n", info.hblks);
    printf("Bytes in mapped regions (hblkhd):      %d\n", info.hblkhd);
    printf("Max. total allocated space (usmblks):  %d\n", info.usmblks);
    printf("Free bytes held in fastbins (fsmblks): %d\n", info.fsmblks);
    printf("Total allocated space (uordblks):      %d\n", info.uordblks);
    printf("Total free space (fordblks):           %d\n", info.fordblks);
    printf("Topmost releasable block (keepcost):   %d\n", info.keepcost);
    printf("=== malloc_info ===\n");
    malloc_info(0, stdout);
    printf("=== malloc_info ===\n");
    malloc_stats();
    printf("\n");
}

int main(int argc, char* argv[])
{
    print_mallinfo();
    void * block = malloc(1024 * 1042);
    print_mallinfo();
    free(block);
    print_mallinfo();

    return EXIT_SUCCESS;
}