#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "app.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "\n\033[31;1m---------------Incorrect Arguments------------------\033[0m\n"
        "\nUsage:\n"
        "  %s -c --output <archive> <file1> [file2 ...]\n"
        "  %s -d --input  <archive> [file1 ...]\n"
        "  %s --help\n\n",
        prog, prog, prog);
}

static void print_help(const char *prog) {
    fprintf(stderr,
        "\nSyntax:\n\n"
        "  \033[37;42m%s -c --output <archive> <file1> [file2 ...]\033[0m\n"
        "      Compresses each specified file into the given archive\n\n"
        "  \033[37;42m%s -d --input  <archive> [file1 ...]\033[0m\n"
        "      Extracts all files from the archive, or only those listed\n\n"
        "  \033[37;42m%s --help\033[0m\n"
        "      Display this help message\n\n",
        prog, prog, prog);
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    int mode = -1; // режим работы   
    const char *archive = NULL; // путь к файлу архива
    char **files = NULL; // массив указателей на имена файлов
    int file_count = 0; // кол-во файлов в files[]

    if (strcmp(argv[1], "-c") == 0)      mode = 1;
    else if (strcmp(argv[1], "-d") == 0) mode = 0;
    else { print_usage(argv[0]); return 1; }

    for (int i = 2; i < argc; i++) {
        if (((mode == 1 && strcmp(argv[i], "--output") == 0) || 
            (mode == 0 && strcmp(argv[i], "--input")  == 0)) 
            && i+1 < argc) {
            archive = argv[++i];
        } else {
            files = realloc(files, sizeof(char*) * (file_count + 1));
            files[file_count++] = argv[i];
        }
    }

    if (!archive || (mode == 1 && file_count == 0)) {
        print_usage(argv[0]);
        free(files);
        return 1;
    }

    int rc;
    if (mode == 1) {
        rc = AppCompress(archive, files, file_count);
    } else {
        rc = AppDecompress(archive, files, file_count);
    }

    free(files);
    return rc;
}
