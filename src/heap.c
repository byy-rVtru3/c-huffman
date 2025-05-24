#include <stdlib.h>
#include "heap.h"

MinHeap *HeapCreate(size_t capacity) { // create
    MinHeap *heap = malloc(sizeof(MinHeap));
    heap->data = malloc(sizeof(tree_node_t *) * capacity);
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void HeapDestroy(MinHeap *heap) { // destroy
    free(heap->data);
    free(heap);
}

static void sift_up(MinHeap *heap, size_t i) { // после вставки проверяем что у родителей частота <= частоты детей
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (heap->data[parent]->freq <= heap->data[i]->freq) break;
        tree_node_t *tmp = heap->data[parent];
        heap->data[parent] = heap->data[i];
        heap->data[i] = tmp;
        i = parent;
    }
}

static void sift_down(MinHeap *heap, size_t i) {  // после удаления корня (на его место втает последний элемент массива) опускаем элемент вниз
    for (;;) {                                    // пока частото детей не станет >= частоты текущего узла
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t smallest = i;

        if (left < heap->size && heap->data[left]->freq < heap->data[smallest]->freq)
            smallest = left;
        if (right < heap->size && heap->data[right]->freq < heap->data[smallest]->freq)
            smallest = right;

        if (smallest == i) break;

        tree_node_t *tmp = heap->data[smallest];
        heap->data[smallest] = heap->data[i];
        heap->data[i] = tmp;
        i = smallest;
    }
}

void HeapInsert(MinHeap *heap, tree_node_t *node) { // вставили + sift_up
    if (heap->size == heap->capacity) return; 
    heap->data[heap->size] = node;
    sift_up(heap, heap->size);
    heap->size++;
}

tree_node_t *HeapExtractMin(MinHeap *heap) { // достали + sift_down
    if (heap->size == 0) return NULL;
    tree_node_t *min = heap->data[0];
    heap->size--;
    heap->data[0] = heap->data[heap->size];
    sift_down(heap, 0);
    return min;
}
