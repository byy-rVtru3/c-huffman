// include/progress.h
#ifndef PROGRESS_H
#define PROGRESS_H

#include <stddef.h>

void ProgressInit(const char *task, size_t total);
void ProgressUpdate(size_t current);
void ProgressFinish(void);

#endif // PROGRESS_H
