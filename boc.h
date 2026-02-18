#ifndef BOC_H
#define BOC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>

// TODO(Alex S): Use dynamic allocation

#define MAX_STR_LEN 1024
#define STRINGBUILDER_DEFAULT_LENGTH 10240
#define MAX_PARAM_COUNT 128

#define MAX_SOURCES 1024
#define MAX_LIBS 1024
#define MAX_LIB_DIRS 1024
#define MAX_INCLUDES 1024
#define MAX_EXECS 128

typedef char boc_string[MAX_STR_LEN];

typedef struct boc_stringbuilder
{
    char *data;
    size_t count;
    size_t capacity;
} boc_stringbuilder;

typedef struct boc_clargs
{
    boc_string command;
    boc_string params[MAX_PARAM_COUNT];
    char param_count;
} boc_clargs;

typedef struct boc_exec
{
    const char *path;

    const char *sources[MAX_SOURCES];
    int source_count;

    const char *includes[MAX_INCLUDES];
    int include_count;

    const char *libs[MAX_LIBS];
    int lib_count;

    const char *lib_dirs[MAX_LIB_DIRS];
    int lib_dir_count;
} boc_exec;

typedef struct boc
{
    int is_dry_run;

    // TODO(Alex S): These should ABSOLUTELY be per executable. Whoops...
    int debug_symbols;
    int wall;
    int wextra;
    int werror;

    boc_exec execs[MAX_EXECS];
    int exec_count;
    int current_exec;
} boc;

#define BOC_ASSERT(condition, msg) do {if (!(condition)) boc_crash_with_error(msg, __LINE__);} while (0)
void boc_crash_with_error(const char *error, int line_number);

void _boc_build_linux();
int _boc_compile_boc_c();

void boc_add_exec(const char *path);
void boc_add_src(const char *path);
void boc_add_lib(const char *name);
void boc_add_lib_dir(const char *path);
void boc_add_include(const char *path);

void boc_flag_debug_symbols();
void boc_flag_wall();
void boc_flag_wextra();
void boc_flag_werror();

void boc_dry_run();

void boc_stringbuilder_init(boc_stringbuilder *sb);
void boc_stringbuilder_destroy(boc_stringbuilder *sb);
void boc_stringbuilder_push(boc_stringbuilder *sb, const char *fmt, ...);
char *boc_stringbuilder_to_str(boc_stringbuilder *sb);

boc_clargs boc_parse_clargs(int argc, const char **argv);
int boc_clargs_command(boc_clargs *clargs, const char *command);
int boc_clargs_param(boc_clargs *clargs, const char *param);

// TODO(Alex S): Make this const boc *b? The user doesn't need to edit it... Should be readonly
// In fact, maybe they shouldn't even get boc *b. Maybe boc_info *b which contains only things they may need??
// Who knows...
int boc_main(boc *b);

#ifndef BOC_NO_IMPLEMENTATION

static boc _BOC_INTERNAL_BOC_STRUCT = {0};

#ifndef BOC_NO_MAIN

int main(int argc, const char **argv)
{
    // Check if boc.c has changed since last compilation
    // If so, recompile it, call the recompiled version, then exit!

    {
        time_t boc_c_last_modified_time = 0;
        time_t boc_h_last_modified_time = 0;
        time_t boc_exec_last_modified_time = 0;
        
        struct stat st;
        
        BOC_ASSERT(stat("boc", &st) == 0, "Failed to stat boc");
        boc_exec_last_modified_time = st.st_mtime;
        BOC_ASSERT(stat("boc.c", &st) == 0, "Failed to stat boc.c");
        boc_c_last_modified_time = st.st_mtime;

        #ifdef BOC_USE_LOCAL_HEADER
        BOC_ASSERT(stat("boc.h", &st) == 0, "Failed to stat boc.h (locally)");
        boc_h_last_modified_time = st.st_mtime;
        #else
        BOC_ASSERT(stat("/usr/local/include/boc.h", &st) == 0, "Failed to stat /usr/local/include/boc.h (globally)");
        boc_h_last_modified_time = st.st_mtime;
        #endif // BOC_USE_LOCAL_HEADER
        
        double h_edit_diff = difftime(boc_exec_last_modified_time, boc_h_last_modified_time);
        double c_edit_diff = difftime(boc_exec_last_modified_time, boc_c_last_modified_time);
        int must_recompile = (h_edit_diff < 0.0) || (c_edit_diff < 0.0);

        if (h_edit_diff < 0.0 && c_edit_diff < 0.0)
        {
            printf("boc.c and boc.h have been updated!\n");
        }
        else if (c_edit_diff < 0.0)
        {
            printf("boc.c has been updated!\n");
        }
        else if (h_edit_diff < 0.0)
        {
            printf("boc.h has been updated!\n");
        }
        
        if (must_recompile)
        {
            printf("Recompiling boc...\n\n");
            BOC_ASSERT(_boc_compile_boc_c() == 0, "\n\nRecompilation failed... Is there an error in boc.c?"); // TODO(Alex S): Check if this actually succeeded? Otherwise infinite loop...
            system("./boc");
            return 0;
        }
    }

    // Setup global boc struct
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    memset(b, 0, sizeof(boc));
    b->current_exec = -1;

    // Call userspace function boc_main
    int ret = boc_main(b);
    
    // Return if there was a boc_main error
    if (ret) return ret;

    // Actually build using the user's stuff
    _boc_build_linux();

    printf("\n");

    return 0;
}

#endif // BOC_NO_MAIN

void boc_crash_with_error(const char *error, int line_number)
{
    if (error) fprintf(stderr, "Error on line %d: %s\n", line_number, error);
    else fprintf(stderr, "Unknown error on line %d\n", line_number);
    exit(-1);
}

void _boc_build_linux()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    // Build each executable independently

    for (int i = 0; i < b->exec_count; i++)
    {
        boc_exec *e = &b->execs[i];
        boc_stringbuilder sb = {0};
        
        printf("Building %s\n", e->path);

        // Build command string

        boc_stringbuilder_init(&sb);
        boc_stringbuilder_push(&sb, "gcc -o \"%s\"", e->path);

        // Add sources

        printf("\tSources:\n");
        for (int j = 0; j < e->source_count; j++)
        {
            printf("\t\t%s\n", e->sources[j]);
            boc_stringbuilder_push(&sb, " \"%s\"", e->sources[j]);
        }

        // Add include directories
        printf("\tIncludes:\n");
        for (int j = 0; j < e->include_count; j++)
        {
            printf("\t\t%s\n", e->includes[j]);
            boc_stringbuilder_push(&sb, " -I\"%s\"", e->includes[j]);
        }

		// Add library include paths
        printf("\tLibrary include paths:\n");
        for (int j = 0; j < e->lib_dir_count; j++)
        {
            printf("\t\t%s\n", e->lib_dirs[j]);
            boc_stringbuilder_push(&sb, " -L\"%s\"", e->lib_dirs[j]);
        }

        // Add libraries
        printf("\tLibraries:\n");
        for (int j = 0; j < e->lib_count; j++)
        {
            printf("\t\t%s\n", e->libs[j]);
            boc_stringbuilder_push(&sb, " -l\"%s\"", e->libs[j]);
        }

        // Add debug flags:
        printf("\tFlags\n");
        if (b->debug_symbols)
        {
            printf("\t\t-g Debug symbols\n");
            boc_stringbuilder_push(&sb, " -g");
        }
        if (b->wall)
        {
            printf("\t\t-Wall All warnings\n");
            boc_stringbuilder_push(&sb, " -Wall");
        }
        if (b->wextra)
        {
            printf("\t\t-Wextra Extra warnings\n");
            boc_stringbuilder_push(&sb, " -Wextra");
        }
        if (b->werror)
        {
            printf("\t\t-Werror Warnings are errors\n");
            boc_stringbuilder_push(&sb, " -Werror");
        }

        // Execute (and print) command

        char *cmd = boc_stringbuilder_to_str(&sb);
        printf("Compile command: %s\n\n", cmd);
        if (!b->is_dry_run)
        {
            BOC_ASSERT(system(cmd) == 0, "Failed to compile executable");
            printf("\nCompiled successfully...\n");
        }
        free(cmd);

        printf("\n");

        boc_stringbuilder_destroy(&sb);
    }
}

int _boc_compile_boc_c()
{
    // NOTE(Alex S): IF THIS EVER GETS CHANGED, REMEMBER TO CHANGE first_build.sh
    return system("gcc -o boc boc.c");
}

void boc_add_exec(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->exec_count < MAX_EXECS, "Reached maximum number of executables!\n");
    
    boc_exec *e = &b->execs[b->exec_count];
    b->current_exec = b->exec_count;
    b->exec_count++;

    e->path = path;
}

void boc_add_src(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->source_count < MAX_SOURCES, "Reached maximum number of sources!\n");

    e->sources[e->source_count] = path;
    e->source_count++;
}

void boc_add_lib(const char *name)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    BOC_ASSERT(name, "name is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->lib_count < MAX_SOURCES, "Reached maximum number of libs!\n");

    e->libs[e->lib_count] = name;
    e->lib_count++;
}

void boc_add_lib_dir(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->lib_dir_count < MAX_SOURCES, "Reached maximum number of lib dirs!\n");

    e->lib_dirs[e->lib_dir_count] = path;
    e->lib_dir_count++;
}

void boc_add_include(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->include_count < MAX_INCLUDES, "Reached maximum number of includes!\n");
    
    e->includes[e->include_count] = path;
    e->include_count++;
}

void boc_flag_debug_symbols()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    b->debug_symbols = 1;
}

void boc_flag_wall()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    b->wall = 1;
}

void boc_flag_wextra()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    b->wextra = 1;
}

void boc_flag_werror()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    b->werror = 1;
}

void boc_dry_run()
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    b->is_dry_run = 1;
}

void boc_stringbuilder_init(boc_stringbuilder *sb)
{
    BOC_ASSERT(sb, "No stringbuilder provided");
    sb->data = malloc(STRINGBUILDER_DEFAULT_LENGTH);
    BOC_ASSERT(sb->data, "Failed to allocate stringbuilder buffer");
    sb->capacity = STRINGBUILDER_DEFAULT_LENGTH;
    sb->count = 0;
}

void boc_stringbuilder_destroy(boc_stringbuilder *sb)
{
    BOC_ASSERT(sb, "No stringbuilder provided");
    free(sb->data);
    sb->capacity = 0;
    sb->count = 0;
}

void boc_stringbuilder_push(boc_stringbuilder *sb, const char *fmt, ...)
{
    BOC_ASSERT(sb, "No stringbuilder provided");
    BOC_ASSERT(fmt, "No format string provided");

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args, args_copy);

    // ret is number of bytes vsnprintf wants to write, excluding null terminator
    // We need ret + 1 free bytes to write the full string without truncating
    // Only add + ret to the count, because the null terminator should be overwritten by the NEXT vsnprintf call
    // When calling to_str, there is probably a null terminator already, so it's checked for then
    // TODO(Alex S): Actually check that lol
    int ret = vsnprintf(NULL, 0, fmt, args_copy);

    BOC_ASSERT(ret + 1 <= sb->capacity - sb->count, "Not enough space in stringbuilder for formatted string");

    vsnprintf(sb->data + sb->count, sb->capacity - sb->count, fmt, args); // Guaranteed to succeed now

    sb->count += ret;

    va_end(args);
    va_end(args_copy);
}

char *boc_stringbuilder_to_str(boc_stringbuilder *sb)
{
    BOC_ASSERT(sb, "No stringbuilder provided");

    // If it ends in a null terminator, just copy data to new string
    // If it DOESN'T end in a null terminator, allocate extra byte for it

    if (sb->data[sb->count - 1] == '\0')
    {
        char *str = malloc(sb->count);
        memcpy(str, sb->data, sb->count);
        return str;
    }
    else
    {
        char *str = malloc(sb->count + 1);
        memcpy(str, sb->data, sb->count);
        str[sb->count] = '\0';
        return str;
    }
}

boc_clargs boc_parse_clargs(int argc, const char **argv)
{
    boc_clargs clargs = {0};

    int has_command = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        size_t len = strlen(arg);
        if (len == 0) continue;
        BOC_ASSERT(len + 1 < MAX_STR_LEN, "Command line argument is longer than boc max string length...");

        int is_param = (arg[0] == '-');

        if (!has_command && !is_param)
        {
            memcpy(clargs.command, arg, len);
            has_command = 1;
        }
        else if (is_param)
        {
            BOC_ASSERT(clargs.param_count < MAX_PARAM_COUNT, "Too many parameters...");
            memcpy(clargs.params[clargs.param_count], arg, len);
            clargs.param_count++;
        }
        else // has_command && !is_param
        {
            BOC_ASSERT(0, "Trying to run with multiple commands...");
        }
    }

    return clargs;
}

int boc_clargs_command(boc_clargs *clargs, const char *command)
{
    BOC_ASSERT(clargs, "No clargs provided...");
    BOC_ASSERT(command, "No command provided...");

    return (strcmp(clargs->command, command) == 0);
}

int boc_clargs_param(boc_clargs *clargs, const char *param)
{
    BOC_ASSERT(clargs, "No clargs provided...");
    BOC_ASSERT(param, "No param provided...");

    for (int i = 0; i < clargs->param_count; i++)
    {
        if (strcmp(clargs->params[i], param) == 0) return 1;
    }

    return 0;
}

#endif // BOC_NO_IMPLEMENTATION
#endif // BOC_H
