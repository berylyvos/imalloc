#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define HEAP_CAP 640000
#define CHUNK_LIST_CAP 1024

typedef struct {
    void *start;
    size_t size;
} Chunk;

typedef struct {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

char heap[HEAP_CAP] = {0};
size_t heap_size = 0;

void *heap_alloc(size_t sz) 
{
    assert(heap_size + sz <= HEAP_CAP);
    void *res = heap + heap_size;
    heap_size += sz;
    return res;  
}

void heap_free(void *ptr) 
{
    (void)ptr;
}

void heap_collect()
{
}

int main()
{
    char *root = heap_alloc(26);
    for (int i = 0; i < 26; ++i) {
        root[i] = i + 'A';
    }

    return 0;
}
