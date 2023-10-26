#include <stdio.h>
#include "heap.h"

#define N 10

void *ptrs[N] = {0};

void test_heap_alloc_free()
{
    for (int i = 0; i < N; ++i) {
        ptrs[i] = heap_alloc(i);
    }

    for (int i = 0; i < N / 2; ++i) {
        heap_free(ptrs[i]);
    }

    heap_alloc(10);
}

void test_scan_heap_pointers()
{
    printf("\nscan heap pointers ---------------\n");
    size_t count = 0;
    for (size_t i = 0; i < alloced_chunks.count; ++i) {
        for (size_t j = 0; j < alloced_chunks.chunks[i].size; ++j) {
            uintptr_t *p = (uintptr_t*) (*(alloced_chunks.chunks[i].start + j));
            if (heap <= p && p <= heap + HEAP_CAP_WORDS) {
                printf("DETECTED HEAP POINTER: %p\n", (void*) p);
                ++count;
            }
        }
    }
    printf("Detected %zu heap pointers\n", count);
}

typedef struct Node Node;
struct Node {
    char x;
    Node *left, *right;
};

const size_t level_max = 3;

Node *generate_tree(size_t level_cur)
{
    if (level_cur < level_max) {
        Node *root = heap_alloc(sizeof(Node));
        root->x = level_cur + 'a';
        root->left = generate_tree(level_cur + 1);
        root->right = generate_tree(level_cur + 1);
        return root;
    } else {
        return NULL;
    }
}

void destroy_tree(Node *root)
{
    if (root != NULL) {
        destroy_tree(root->left);
        destroy_tree(root->right);
        heap_free(root);
        root = NULL;
    }
}

int main()
{
    Node *root = generate_tree(0);
    printf("root: %p\n", (void*) root);
    heap_dump();

    test_scan_heap_pointers();

    destroy_tree(root);
    printf("\nafter destroying tree\n");
    heap_dump();

    test_heap_alloc_free();
    printf("\ntest_heap_alloc_free()\n");
    heap_dump();

    return 0;
}
