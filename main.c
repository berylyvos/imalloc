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

    destroy_tree(root);
    printf("\nafter destroying tree...\n");
    heap_dump();

    test_heap_alloc_free();
    printf("\ntest_heap_alloc_free()\n");
    heap_dump();

    return 0;
}
