#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "app.h"
#include "huffman.h"
#include "bitio.h"
#include "progress.h"

static const char EGORIK[6] = { 'E','G','O','R','I','K' };

typedef struct {
    char       *name;
    uint64_t    offset;
    uint64_t    comp_size;
    uint64_t    orig_size;
    size_t      freq[256];
} FileIndex;

static long get_file_size(FILE *f) {
    long cur = ftell(f);
    if (cur < 0) return -1;
    fseek(f, 0, SEEK_END); // перемещаем курсор в конец файла
    long sz = ftell(f); // узнаем размер
    fseek(f, cur, SEEK_SET); // возвращаем курсор 
    return sz;
}

int AppCompress(const char *archive, char *files[], int file_count) {
    FileIndex *idx = calloc(file_count, sizeof(FileIndex));
    for (int i = 0; i < file_count; i++) {
        idx[i].name = files[i];
    }

    FILE *out = fopen(archive, "wb");
    if (!out) { perror("\033[31;1mfopen archive\033[0m"); free(idx); return 2; }
    uint32_t cnt32 = (uint32_t)file_count;
    size_t hdr_size = 6 + 4; // 6 байт для EGORIK, 4 байта (uint32_t) с числом файлов
    for (int i = 0; i < file_count; i++) {
        hdr_size += 2 + strlen(idx[i].name) + 8 + 8 + 8; // 2 длина имени, strlen() байт имени
    }                                                    // 8 байт offset + comp_size + orig_size
    for (size_t i = 0; i < hdr_size; i++) fputc(0, out); // заполняем нулями, потом перезаписываем реальными данными

    for (int i = 0; i < file_count; i++) {
        FILE *fin = fopen(idx[i].name, "rb");
        if (!fin) { perror("\033[31;1mfopen input\033[0m"); fclose(out); free(idx); return 3; }

        long sz = get_file_size(fin);
        idx[i].orig_size = (sz < 0 ? 0 : (uint64_t)sz); // получаем размер

        memset(idx[i].freq, 0, sizeof(idx[i].freq)); // частота для каждого байта 0
        unsigned char buf;
        ProgressInit("Counting", idx[i].orig_size);
        while (fread(&buf, 1, 1, fin) == 1) { // побайтное чтение + инкрементирование freq
            idx[i].freq[buf]++;
            ProgressUpdate(1);
        }
        ProgressFinish();
        rewind(fin); // возвращаемся в начало

        tree_node_t *root = BuildHuffmanTree(256, idx[i].freq); // строим дерево для данного файла
        code_t codes[256] = {{0,0}};
        GenerateCodes(root, codes); // генерируем коды

        idx[i].offset = ftell(out); // запоминаем с какого байта начнется блок текущего файла

        for (int b = 0; b < 256; b++) {
            fwrite(&idx[i].freq[b], sizeof(size_t), 1, out);
        } // записываем таблицу частот

        BitWriter *bw = BitWriterCreate(out); // начинаем побитовое кодирование
        ProgressInit("Encoding", idx[i].orig_size);
        while (fread(&buf, 1, 1, fin) == 1) {
            code_t c = codes[buf];
            for (int bit = c.length - 1; bit >= 0; bit--) {
                BitWriterWrite(bw, (c.bits >> bit) & 1);
            }
            ProgressUpdate(1);
        }
        BitWriterFlush(bw);
        ProgressFinish();
        BitWriterDestroy(bw);

        idx[i].comp_size = ftell(out) - idx[i].offset; // размер (текущий - запомненный offset)

        FreeTree(root);
        fclose(fin);
    }

    fseek(out, 0, SEEK_SET); // возвращаемся в начало и запичываем:
    fwrite(EGORIK, 1, 6, out); // 6 байт EGORIK
    fwrite(&cnt32, sizeof(cnt32), 1, out); // 4 байта file_count
    for (int i = 0; i < file_count; i++) { // для каждого файла:
        uint16_t nl = (uint16_t)strlen(idx[i].name); // 2 байта длины имени
        fwrite(&nl, sizeof(nl), 1, out); 
        fwrite(idx[i].name, 1, nl, out); // само имя
        fwrite(&idx[i].offset,   sizeof(uint64_t), 1, out); // offset + сайзы
        fwrite(&idx[i].comp_size, sizeof(uint64_t), 1, out);
        fwrite(&idx[i].orig_size, sizeof(uint64_t), 1, out);
    }

    fclose(out);

    for (int i = 0; i < file_count; i++) {
        uint64_t in_size  = idx[i].orig_size;
        uint64_t out_size = idx[i].comp_size;
        double ratio_pct;
        if (in_size > 0) {
            ratio_pct = (out_size * 100.0) / in_size;
        } else {
            ratio_pct = 0.0;
        }
        printf("\n\033[37;42mCompression finished for %s\033[0m\n",
               idx[i].name);
        printf("Input file size: %llu bytes\n",
               (unsigned long long)in_size);
        printf("Output file size: %llu bytes\n",
               (unsigned long long)out_size);
        printf("Compression ratio: %.2f%%\n", ratio_pct);
    }

    free(idx);
    return 0;
}

int AppDecompress(const char *archive, char *files[], int file_count) {
    FILE *in = fopen(archive, "rb");
    if (!in) { perror("\033[31;1mfopen archive\033[0m"); return 1; }

    char mag[6];
    fread(mag, 1, 6, in); 
    if (memcmp(mag, EGORIK, 6) != 0) { // читаем первые 6 байт и сравниваем
        fprintf(stderr, "\033[31;1mNot a EGORIK archive\033[0m\n");
        fclose(in);
        return 2;
    }

    uint32_t cnt32;
    fread(&cnt32, sizeof(cnt32), 1, in); // читаем кол-во файлов
    int cnt = (int)cnt32;
    FileIndex *idx = calloc(cnt, sizeof(FileIndex));
    for (int i = 0; i < cnt; i++) {
        uint16_t nl;
        fread(&nl, sizeof(nl), 1, in);
        idx[i].name = malloc(nl + 1);
        fread(idx[i].name, 1, nl, in);
        idx[i].name[nl] = '\0';
        fread(&idx[i].offset,   sizeof(uint64_t), 1, in);
        fread(&idx[i].comp_size, sizeof(uint64_t), 1, in);
        fread(&idx[i].orig_size, sizeof(uint64_t), 1, in);
    } // восстанвливаем длину имени, имя + \0 + читаем offset и сайзы

    for (int i = 0; i < cnt; i++) {
        if (file_count > 0) {
            int ok = 0;
            for (int j = 0; j < file_count; j++) {
                if (strcmp(files[j], idx[i].name) == 0) { ok = 1; break; }
            }
            if (!ok) continue;
        } // проверяем входит ли в files[] текущее имя и обрабатываем; иначе пропускаем
        fseek(in, idx[i].offset, SEEK_SET); // перемещаемся к текщуему блоку

        size_t freq[256]; 
        for (int b = 0; b < 256; b++) {
            fread(&freq[b], sizeof(size_t), 1, in);
        } // считываем таблицу частот

        tree_node_t *root = BuildHuffmanTree(256, freq); // восстанавливаем дерево Хаффмана
        BitReader *br = BitReaderCreate(in); 
        ProgressInit("Decoding", idx[i].orig_size);

        FILE *out = fopen(idx[i].name, "wb");
        tree_node_t *node = root;
        int bit;
        size_t done = 0;
        while (done < idx[i].orig_size && (bit = BitReaderRead(br)) != -1) {
            node = bit ? node->right : node->left;
            if (!node->left && !node->right) { // дошли до листа -> выводим sym
                fputc(node->sym, out);
                node = root;
                done++;
                ProgressUpdate(1);
            }
        }
        ProgressFinish();

        BitReaderDestroy(br);
        FreeTree(root);
        fclose(out);
    }

    for (int i = 0; i < cnt; i++) {
        free(idx[i].name);
    }
    free(idx);
    fclose(in);
    return 0;
}
