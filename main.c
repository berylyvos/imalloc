#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WORD sizeof(uintptr_t)
#define HEAP_CAP 640000

static_assert(HEAP_CAP % WORD == 0, 
              "The heap capacity is not divisible by "
              "the size of the pointer. Of the platform.");
uintptr_t heap[HEAP_CAP / WORD] = {0};

#define CHUNK_LIST_CAP 1024

#define UNIMPLEMENTED \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented yet\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

typedef struct {
    uintptr_t *start;
    size_t size;
} Chunk;

// allocated or freed list of chunks
typedef struct {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

int chunk_list_find(const Chunk_List *list, uintptr_t *ptr)
{
    int l = 0, r = list->count - 1;
    while (l < r) {
        int mid = (l + r) >> 1;
        if (list->chunks[mid].start >= ptr) {
            r = mid;
        } else {
            l = mid + 1;
        }
    }
    
    if (r >= 0 && list->chunks[r].start == ptr) {
        return r;
    }
    return -1;
}

void chunk_list_insert(Chunk_List *list, void *start, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    list->chunks[list->count].start = start;
    list->chunks[list->count].size  = size;

    for (size_t i = list->count;
            i > 0 && list->chunks[i - 1].start > list->chunks[i].start;
            --i) {
        const Chunk t = list->chunks[i];
        list->chunks[i] = list->chunks[i - 1];
        list->chunks[i - 1] = t;
    }

    list->count++;
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
    assert(index < list->count);
    for (size_t i = index; i < list->count - 1; ++i) {
        list->chunks[i] = list->chunks[i + 1];
    }
    list->count--;
}

void chunk_list_merge(Chunk_List *dst, Chunk_List *src)
{
    dst->count = 0;

    for (size_t i = 0; i < src->count; ++i) {
        const Chunk chunk = src->chunks[i];

        if (dst->count > 0) {
            Chunk *top_chunk = &dst->chunks[dst->count - 1];

            if (top_chunk->start + top_chunk->size == chunk.start) {
                top_chunk->size += chunk.size;
            } else {
                chunk_list_insert(dst, chunk.start, chunk.size);
            }
        } else {
            chunk_list_insert(dst, chunk.start, chunk.size);
        }
    }
}

void chunk_list_dump(const Chunk_List *list)
{
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; ++i) {
        printf("  start: %p, size: %zu\n",
                (void*) list->chunks[i].start,
                list->chunks[i].size);
    }
}

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
    .count = 1,
    .chunks = {
        [0] = {.start = heap, .size = sizeof(heap)}
    },
};
Chunk_List tmp_chunks = {0};

void *heap_alloc(size_t bytes) 
{
    const size_t size = (bytes + WORD - 1) / WORD;

    if (size > 0) {
        chunk_list_merge(&tmp_chunks, &freed_chunks);
        freed_chunks = tmp_chunks;

        for (size_t i = 0; i < freed_chunks.count; ++i) {
            const Chunk chunk = freed_chunks.chunks[i];
            if (chunk.size >= size) {
                chunk_list_remove(&freed_chunks, i);

                const size_t tail_size = chunk.size - size;
                chunk_list_insert(&alloced_chunks, chunk.start, size);

                if (tail_size > 0) {
                    chunk_list_insert(&freed_chunks, chunk.start + size, tail_size);
                }

                return chunk.start;
            }
        }
    }

    return NULL;
}

void heap_free(void *ptr) 
{
    if (ptr != NULL) {
        const int index = chunk_list_find(&alloced_chunks, ptr);
        assert(index >= 0);
        chunk_list_insert(&freed_chunks, 
                          alloced_chunks.chunks[index].start, 
                          alloced_chunks.chunks[index].size);
        chunk_list_remove(&alloced_chunks, (size_t) index);
    }
}

void heap_collect()
{
    UNIMPLEMENTED;
}

#define N 10

void *ptrs[N] = {0};

int main()
{
    for (int i = 0; i < N; ++i) {
        ptrs[i] = heap_alloc(i);
    }

    for (int i = 0; i < N / 2; ++i) {
        heap_free(ptrs[i]);
    }

    heap_alloc(10);

    chunk_list_dump(&alloced_chunks);
    chunk_list_dump(&freed_chunks);

    return 0;
}
