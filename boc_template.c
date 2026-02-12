#include "boc.h"

int boc_main(boc *b)
{
    boc_add_exec("a.out");
    boc_add_src("main.c");
    return 0;
}