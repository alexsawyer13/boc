#define BOC_NO_MAIN
#define BOC_USE_LOCAL_HEADER
#include "boc.h"

const char *boc_h_location = "/usr/local/include/boc.h";
const char *boc_c_location = "/usr/local/src/boc_template.c";

int main(int argc, const char **argv)
{
    struct stat st;

    // Make sure boc.h and boc_template.c actually exist...
    BOC_ASSERT(stat(boc_h_location, &st) == 0, "boc.h doesn't exist in /usr/local/include, you're doing something very wrong...");
    BOC_ASSERT(stat(boc_c_location, &st) == 0, "boc_template.c doesn't exist in /usr/local/src, you're doing something very wrong...");
    
    // Parse clargs

    boc_clargs cl = boc_parse_clargs(argc, argv);

    // Switch through the commands

    if (boc_clargs_command(&cl, "init") || boc_clargs_command(&cl, ""))
    {
        printf("Initialising boc in current directory\n");

        // Check if we ALREADY have a boc repository...
        int has_boc_files = (stat("boc", &st) == 0) || (stat("boc.c", &st) == 0) || (stat("boc.h", &st) == 0);
        
        // If we already have boc files, then quit unless -f is specified

        if (has_boc_files)
        {
            BOC_ASSERT(boc_clargs_param(&cl, "-f"), "Running boc_init on a directory which ALREADY contains boc files without -f flag...");
            printf("Directory already contains boc files, but -f is specified. Going on anyway...\n");
        }

        // Copy boc.c template in
        BOC_ASSERT(system("cp /usr/local/src/boc_template.c ./boc.c") == 0, "Failed to copy boc_template.c to directory");

        // Compile boc.c
        BOC_ASSERT(_boc_compile_boc_c() == 0, "Failed to compile boc.c, it will be left in the directory");

        printf("Success!\n");
    }
    else if (boc_clargs_command(&cl, "compile"))
    {
        printf("Compiling boc.c in current directory\n");
        
        BOC_ASSERT(_boc_compile_boc_c() == 0, "Failed to compile boc.c");
    }
    else
    {
        BOC_ASSERT(0, "Unknown command...");
    }

    return 0;
}