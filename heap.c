#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "heap.h"

uintptr_t heap[HEAP_CAP_WORDS] = {0};
uintptr_t *stack_base = 0;

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
    .count = 1,
    .chunks = {
        [0] = {.start = heap, .size = sizeof(heap)}
    },
};
Chunk_List tmp_chunks = {0};

bool reachable_chunks[CHUNK_LIST_CAP] = {0};
void *to_free_chunks[CHUNK_LIST_CAP] = {0};
size_t to_free_chunks_count = 0;

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

void chunk_list_dump(const Chunk_List *list, const char *name)
{
    printf("%s Chunks (%zu):\n", name, list->count);
    for (size_t i = 0; i < list->count; ++i) {
        printf("  start: %p, size: %zu\n",
                (void*) list->chunks[i].start,
                list->chunks[i].size);
    }
}

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

static void mark_region(uintptr_t *start, uintptr_t *end)
{
    for (; start < end; start++) {
        uintptr_t *p = (uintptr_t*) *start;
        for (size_t i = 0; i < alloced_chunks.count; ++i) {
            Chunk chunk = alloced_chunks.chunks[i];
            if (chunk.start <= p && p < chunk.start + chunk.size) {
                if (!reachable_chunks[i]) {
                    printf("REACHED: %p\n", (void*)p);
                    reachable_chunks[i] = true;
                    mark_region(chunk.start, chunk.start + chunk.size);
                }
            }
        }
    }
}

void heap_collect()
{
    uintptr_t *stack_start = (uintptr_t*)__builtin_frame_address(0);
    memset(reachable_chunks, 0, sizeof(reachable_chunks));
    mark_region(stack_start, stack_base + 1);

    to_free_chunks_count = 0;
    for (size_t i = 0; i < alloced_chunks.count; ++i) {
        if (!reachable_chunks[i]) {
            to_free_chunks[to_free_chunks_count++] = alloced_chunks.chunks[i].start;
        }
    }

    for (size_t i = 0; i < to_free_chunks_count; ++i) {
        heap_free(to_free_chunks[i]);
    }
}

void heap_dump()
{
    chunk_list_dump(&alloced_chunks, "Alloced");
    chunk_list_dump(&freed_chunks, "Freed");
}
