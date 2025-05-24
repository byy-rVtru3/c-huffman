#include <stdio.h>
#include "progress.h"

static const char *task_name;
static size_t total = 0;
static size_t last_percent = (size_t)-1;
static size_t processed = 0;

void ProgressInit(const char *task, size_t total_bytes) { // инициализация прогресса
    task_name = task;
    total = total_bytes;
    last_percent = (size_t)-1;
    processed = 0;
    if (total > 0) {
        printf("%s: 0%%", task_name);
    } else {
        printf("%s: processing...", task_name);
    }
    fflush(stdout); 
}

void ProgressUpdate(size_t bytes) {
    if (total == 0) return; // неизвестен общий размер
    processed += bytes;
    size_t percent = (processed * 100) / total;
    if (percent != last_percent) {
        printf("\r%s: %zu%%", task_name, percent); // возвращаем курсор в начало текущей строки 
        fflush(stdout);
        last_percent = percent;
    }
}

void ProgressFinish(void) {
    if (total > 0) {
        printf("\r%s: 100%% \033[37;42mdone\033[0m\n", task_name);
    } else { // = 0
        printf("%s: \033[37;42mdone\033[0m\n", task_name);
    }
    fflush(stdout);
}
