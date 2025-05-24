#include <stdlib.h>
#include "huffman.h"
#include "heap.h"

static void generate_codes_recursive(tree_node_t *node, code_t codes[], unsigned int bits, unsigned int length) {
    if (!node) return;

    if (!node->left && !node->right) {
        codes[(unsigned char)node->sym].bits = bits; // рекурсивно спускаемся и записываем значение в битах по codes['A'].bits = 0b00
        codes[(unsigned char)node->sym].length = length; // рекурсивно спускаемся и записываем длину в битах по codes['A'].length = 2
        return;
    }
    generate_codes_recursive(node->left, codes, bits << 1, length + 1); // 0
    generate_codes_recursive(node->right, codes, (bits << 1) | 1, length + 1); // 1
}

tree_node_t *BuildHuffmanTree(size_t n, size_t freq[]) {
    MinHeap *heap = HeapCreate(n);

    for (size_t i = 0; i < n; i++) { // вставляем в кучу листья для каждого символа с ненулевой частотой
        if (freq[i] > 0) {
            tree_node_t *leaf = malloc(sizeof(tree_node_t));
            leaf->freq = freq[i];
            leaf->sym = (unsigned char)i;
            leaf->left = leaf->right = NULL;
            HeapInsert(heap, leaf);
        }
    }

    if (heap->size == 1) { // только 1 символ во всем файле (специальный случай)
        tree_node_t *only = HeapExtractMin(heap);
        tree_node_t *root = malloc(sizeof(tree_node_t));
        root->freq = only->freq;
        root->sym = 0;
        root->left = only;
        root->right = NULL;
        HeapInsert(heap, root);
    }

    while (heap->size > 1) { // пока в куче >= 2 узлов, извлекаем два минимума о объединяем
        tree_node_t *a = HeapExtractMin(heap);
        tree_node_t *b = HeapExtractMin(heap);
        tree_node_t *parent = malloc(sizeof(tree_node_t));
        parent->freq = a->freq + b->freq; // суммируем частоты
        parent->left = a; 
        parent->right = b;
        parent->sym = 0;
        HeapInsert(heap, parent);
    }

    tree_node_t *root = HeapExtractMin(heap); // последний узел - корень
    HeapDestroy(heap);
    return root;
}

void GenerateCodes(tree_node_t *root, code_t codes[]) { // запуск генерации кодов
    for (int i = 0; i < 256; i++)
        codes[i].length = 0;
    generate_codes_recursive(root, codes, 0, 0);
}

void FreeTree(tree_node_t *root) {
    if (!root) return;
    FreeTree(root->left);
    FreeTree(root->right);
    free(root);
}
