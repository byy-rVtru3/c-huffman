#ifndef HEAP_H
#define HEAP_H

#include "huffman.h"

typedef struct {
    tree_node_t **data;
    size_t size, capacity;
} MinHeap;

MinHeap *HeapCreate(size_t capacity);
void HeapDestroy(MinHeap *h);
void HeapInsert(MinHeap *h, tree_node_t *node);
tree_node_t *HeapExtractMin(MinHeap *h);

#endif