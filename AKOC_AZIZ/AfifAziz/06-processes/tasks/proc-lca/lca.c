#include "lca.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_TREE_DEPTH 100


typedef struct {
    pid_t* path;
    int length;
} ProcessPath;

//получения родительского процесса по его PID
pid_t get_parent(pid_t pid) {
    FILE* fd;
    char path[100];
    char buff[256];

    // Формируем путь к файлу status процесса
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fd = fopen(path, "r");

    if (fd == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Считываем информацию из файла status и извлекаем PID родительского процесса
    while (fgets(buff, sizeof(buff), fd) != NULL) {
        if (sscanf(buff, "PPid: %d", &pid) == 1) {
            fclose(fd);
            return pid;
        }
    }

    fclose(fd);
    return 0;
}

//получения пути процесса в дереве процессов
ProcessPath get_process_path(pid_t pid) {
    ProcessPath result;
    result.path = (pid_t*)calloc(MAX_TREE_DEPTH, sizeof(pid_t));

    int index = 0;
    while (pid != 1) {
        result.path[index++] = pid;
        pid = get_parent(pid);
    }

    result.path[index++] = 1;  // Добавляем корневой процесс (PID 1) в путь
    result.length = index;

    return result;
}

// поиска наименьшего общего предка процессов process_x и process_y
pid_t find_lca(pid_t process_x, pid_t process_y) {
    ProcessPath path_x = get_process_path(process_x);
    ProcessPath path_y = get_process_path(process_y);

    int index_x = path_x.length - 1, index_y = path_y.length - 1;
    // Находим последний общий процесс в путях process_x и process_y
    while (index_x >= 0 && index_y >= 0 && path_x.path[index_x] == path_y.path[index_y]) {
        index_x--;
        index_y--;
    }

    return path_x.path[index_x + 1];  // Возвращаем PID наименьшего общего предка
}
