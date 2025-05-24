#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>

typedef struct tree_node_t {
    size_t freq;
    unsigned char sym;
    struct tree_node_t *left;
    struct tree_node_t *right;
} tree_node_t;

typedef struct { // хранение кода хаффмана для каждого sym
    unsigned int bits;
    unsigned int length;
} code_t;

tree_node_t *BuildHuffmanTree(size_t n, size_t freq[]);
void GenerateCodes(tree_node_t *root, code_t codes[]);
void FreeTree(tree_node_t *root);

#endif 
