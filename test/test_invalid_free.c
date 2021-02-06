#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    int value = 42;
    void * invalid_ptr = &value;
    free(invalid_ptr);

    return EXIT_SUCCESS;
}