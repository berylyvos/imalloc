#include "heap.h"

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
