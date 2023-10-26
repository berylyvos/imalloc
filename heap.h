#ifndef HEAP_H_
#define HEAP_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define UNIMPLEMENTED \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented yet\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

#define HEAP_CAP_BYTES 640000
#define CHUNK_LIST_CAP 1024
#define WORD sizeof(uintptr_t)

static_assert(HEAP_CAP_BYTES % WORD == 0, 
              "The heap capacity is not divisible by "
              "the size of the pointer. Of the platform.");
#define HEAP_CAP_WORDS (HEAP_CAP_BYTES / WORD)

extern uintptr_t heap[HEAP_CAP_WORDS];
extern uintptr_t *stack_base;

void *heap_alloc(size_t bytes);
void heap_free(void *ptr);
void heap_collect();
void heap_dump();

typedef struct {
    uintptr_t *start;
    size_t size;
} Chunk;

typedef struct {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

extern Chunk_List alloced_chunks;
extern Chunk_List freed_chunks;
extern Chunk_List tmp_chunks;

int chunk_list_find(const Chunk_List *list, uintptr_t *ptr);
void chunk_list_insert(Chunk_List *list, void *start, size_t size);
void chunk_list_remove(Chunk_List *list, size_t index);
void chunk_list_merge(Chunk_List *dst, Chunk_List *src);
void chunk_list_dump(const Chunk_List *list, const char *name);

#endif // HEAP_H_
