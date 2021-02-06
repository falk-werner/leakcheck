#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    void * some_data = realloc(NULL, 42);
    some_data = realloc(some_data, 128);
    free(some_data);

    return EXIT_SUCCESS;
}