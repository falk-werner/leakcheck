#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    void * some_data = calloc(1, 42);
    free(some_data);

    return EXIT_SUCCESS;
}