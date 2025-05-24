#include <stdlib.h>
#include <stdio.h>
#include "bitio.h"

BitWriter *BitWriterCreate(FILE *f) { // создаем 
    BitWriter *bw = malloc(sizeof(BitWriter));
    bw->f = f;
    bw->buf = 0;
    bw->pos = 0;
    return bw;
}

void BitWriterWrite(BitWriter *bw, unsigned int bit) { // записываем один бит
    bw->buf = (bw->buf << 1) | (bit & 1); // сдвигаем в старший, записываем в младший
    bw->pos++;
    if (bw->pos == 8) {
        fputc(bw->buf, bw->f); // пишем в файл
        bw->pos = 0;
        bw->buf = 0;
    }
}

void BitWriterFlush(BitWriter *bw) { // дозаписываем неполный байт
    if (bw->pos > 0) {
        bw->buf <<= (8 - bw->pos);
        fputc(bw->buf, bw->f);
        bw->pos = 0;
        bw->buf = 0;
    }
}

void BitWriterDestroy(BitWriter *bw) { // destroy
    free(bw);
}

BitReader *BitReaderCreate(FILE *f) {  // создаем
    BitReader *br = malloc(sizeof(BitReader));
    br->f = f;
    br->pos = 8;
    br->buf = 0;
    return br;
}

int BitReaderRead(BitReader *br) {
    if (br->pos == 8) { // все биты из буфера прочитаны -> загружаем новый байт
        int c = fgetc(br->f);
        if (c == EOF) return -1; // конец файла
        br->buf = (unsigned char)c; // загруженный байт
        br->pos = 0; 
    }
    int bit = (br->buf >> (7 - br->pos)) & 1; // извлекаем бит
    br->pos++;
    return bit;
}

void BitReaderDestroy(BitReader *br) { // destroy
    free(br);
}
