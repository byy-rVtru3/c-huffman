#ifndef APP_H
#define APP_H

int AppCompress(const char *archive, char *files[], int file_count);

int AppDecompress(const char *archive, char *files[], int file_count);

#endif 
