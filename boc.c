#include "boc.h"

#include <stdio.h>

int boc_main(boc *b)
{
    printf("Hello, world!\n");
    printf("%d\n", b->current_exec);

    boc_add_exec("boc");
    boc_add_src("boc.c");

    return 0;
}