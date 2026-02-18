#define BOC_USE_LOCAL_HEADER
#include "boc.h"

#include <stdio.h>

int boc_main(boc *b)
{
    boc_add_exec("boc_init");
    boc_add_src("boc_init.c");
    return 0;
}
