#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Counter {
    char filename[PATH_MAX];
    int counter;
    struct Counter* next;
} Counter;

typedef struct Counters {
    struct Counter* head;
} Counters;

void increment(Counters* counters, char* filename, int value) {
    Counter* current = counters->head;
    while (current != NULL) {
        if (strncmp(current->filename, filename, PATH_MAX) == 0) {
            current->counter += value;
            return;
        }
        current = current->next;
    }
    Counter* new_head = malloc(sizeof(Counter));
    new_head->next = counters->head;
    new_head->counter = value;
    strncpy(new_head->filename, filename, PATH_MAX - 1);
    counters->head = new_head;
}

void print(Counters* counters) {
    Counter* current = counters->head;
    while (current != NULL) {
        printf("%s:%d\n", current->filename, current->counter);
        current = current->next;
    }
}

char** split(char *str, char* delimiter, int* count) {
    char** tokens = malloc(sizeof(char*) * 100);
    int i = 0;

    while (*str != '\0') {
        while (*str != '\0' && strchr(delimiter, *str)) {
            ++str;
        }

        if (*str == '\0') {
            break;
        }

        const char* word_start = str;
        while (*str != '\0' && !strchr(delimiter, *str)) {
            ++str;
        }
        int word_length = str - word_start;
        tokens[i] = malloc(word_length + 1);
        strncpy(tokens[i], word_start, word_length);
        tokens[i][word_length] = '\0';
        ++i;
    }

    *count = i;
    return tokens;
}

void processCommand(Counters* counters, char* command) {
    int count = 0;
    char** tokens = split(command, ";", &count);

    for (int i = 0; i < count; ++i) {
        char* filename;
        sscanf(tokens[i], "echo -n %*s 1>%ms", &filename);
        increment(counters, filename, strlen(tokens[i]) - strlen(filename) - strlen(" 1>") - strlen("echo -n "));
        free(filename);
    }

    for (int i = 0; i < count; ++i) {
        free(tokens[i]);
    }
    free(tokens);
}

int main(int argc, char* argv[]) {
    Counters* counters = malloc(sizeof(Counters));
    counters->head = NULL;

    for (int i = 3; i < argc; ++i) {
        processCommand(counters, argv[i]);
    }

    print(counters);

    Counter* current = counters->head;
    while (current != NULL) {
        Counter* next = current->next;
        free(current);
        current = next;
    }
    free(counters);

    return 0;
}
