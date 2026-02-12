#ifndef BOC_H
#define BOC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Windows.h>

// TODO(Alex S): Use dynamic allocation
#define MAX_SOURCES 1024
#define MAX_INCLUDES 1024
#define MAX_EXECS 128

#define STRINGBUILDER_DEFAULT_LENGTH 10240

typedef struct boc_stringbuilder
{
    char *data;
    size_t count;
    size_t capacity;
} boc_stringbuilder;

typedef struct boc_exec
{
    const char *path;
    const char *sources[MAX_SOURCES];
    int source_count;
    const char *includes[MAX_INCLUDES];
    int include_count;
} boc_exec;

typedef struct boc
{
    boc_exec execs[MAX_EXECS];
    int exec_count;
    int current_exec;
} boc;

#define BOC_ASSERT(condition, msg) do {if (!(condition)) boc_crash_with_error(msg, __LINE__);} while (0)

void boc_crash_with_error(const char *error, int line_number);

void boc_add_exec(const char *path);
void boc_add_src(const char *path);
void boc_add_include(const char *path);

void boc_stringbuilder_init(boc_stringbuilder *sb);
void boc_stringbuilder_destroy(boc_stringbuilder *sb);
void boc_stringbuilder_push_str(boc_stringbuilder *sb, const char *c);
char *boc_stringbuilder_to_str(boc_stringbuilder *sb);

int boc_main(boc *b);

#ifndef BOC_NO_IMPLEMENTATION

static boc _BOC_INTERNAL_BOC_STRUCT;

int main(int argc, const char **argv)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    BOC_ASSERT(b, "Boc internal struct not found");    

    memset(b, 0, sizeof(boc));
    b->current_exec = -1;

    int ret = boc_main(b);
    
    if (ret) return ret;

        boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    BOC_ASSERT(b, "Boc internal struct not found"); 

    // Build each executable independently

    for (int i = 0; i < b->exec_count; i++)
    {
        boc_exec *e = &b->execs[i];
        boc_stringbuilder sb = {0};
        
        printf("Building \"%s\"\n", e->path);

        // Build command string

        boc_stringbuilder_init(&sb);
        boc_stringbuilder_push_str(&sb, "\"C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\Common7\\Tools\\VsDevCmd.bat\" && cl.exe");

        // Add sources

        printf("Sources:\n");
        for (int j = 0; j < e->source_count; j++)
        {
            printf("\t%s\n", e->sources[j]);
            boc_stringbuilder_push_str(&sb, " ");
            boc_stringbuilder_push_str(&sb, e->sources[j]);
        }

        // Add include directories
        
        printf("Includes:\n");
        for (int j = 0; j < e->include_count; j++)
        {
            printf("\t%s\n", e->includes[j]);
        }

        // Print command as test...

        char *cmd = boc_stringbuilder_to_str(&sb);
        printf("%s\n", cmd);

        // system(cmd);

        free(cmd);

        boc_stringbuilder_destroy(&sb);
    }

    return 0;
}

void boc_crash_with_error(const char *error, int line_number)
{
    if (error) fprintf(stderr, "Error on line %d: %s\n", line_number, error);
    else fprintf(stderr, "Unknown error on line %d\n", line_number);
    exit(-1);
}

void boc_add_exec(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    BOC_ASSERT(b, "Boc internal struct not found");    

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
    BOC_ASSERT(b, "Boc internal struct not found");    

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->source_count < MAX_SOURCES, "Reached maximum number of sources!\n");

    e->sources[e->source_count] = path;
    e->source_count++;
}

void boc_add_include(const char *path)
{
    boc *b = &_BOC_INTERNAL_BOC_STRUCT;
    BOC_ASSERT(b, "Boc internal struct not found"); 

    BOC_ASSERT(path, "path is null");

    BOC_ASSERT(b->current_exec >= 0, "No current executable");
    BOC_ASSERT(b->current_exec < b->exec_count, "Exec out of bounds");
    boc_exec *e = &b->execs[b->current_exec];
    
    BOC_ASSERT(e->include_count < MAX_INCLUDES, "Reached maximum number of includes!\n");
    
    e->includes[e->include_count] = path;
    e->include_count++;
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

void boc_stringbuilder_push_str(boc_stringbuilder *sb, const char *c)
{
    BOC_ASSERT(sb, "No stringbuilder provided");
    BOC_ASSERT(c, "No string provided");

    size_t len = strlen(c);
    BOC_ASSERT(sb->count + len <= sb->capacity, "stringbuilder ran out of space");
    memcpy(sb->data + sb->count, c, len);
    sb->count += len;
}

char *boc_stringbuilder_to_str(boc_stringbuilder *sb)
{
    BOC_ASSERT(sb, "No stringbuilder provided");

    char *str = malloc(sb->count + 1);
    memcpy(str, sb->data, sb->count);
    str[sb->count] = '\0';

    return str;
}

#endif // BOC_NO_IMPLEMENTATION

#endif // BOC_H