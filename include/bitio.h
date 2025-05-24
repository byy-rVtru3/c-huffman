#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>

typedef struct {
    FILE *f; // файл для записи
    unsigned char buf; // буфер, до 8 бит
    int pos; // сколько бит лежит в буфере
} BitWriter;

typedef struct {
    FILE *f; // файл для чтения 
    unsigned char buf; 
    int pos; // сколько уже прочитано, при 8 - загружаем новый байт
} BitReader;

BitWriter *BitWriterCreate(FILE *f);
void BitWriterWrite(BitWriter *bw, unsigned int bit);
void BitWriterFlush(BitWriter *bw);
void BitWriterDestroy(BitWriter *bw);

BitReader *BitReaderCreate(FILE *f);
int BitReaderRead(BitReader *br);
void BitReaderDestroy(BitReader *br);

#endif
