#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

enum {
    MAX_ARGS_COUNT = 256,
    MAX_CHAIN_LINKS_COUNT = 256
};

typedef struct {
    char* command;
    uint64_t argc;
    char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
    uint64_t chain_links_count;
    chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;

void create_chain(char* command, chain_t* chain) {
    chain->chain_links_count = 0;
    char* token = strtok(command, " ");

    while (token != NULL && chain->chain_links_count < MAX_CHAIN_LINKS_COUNT) {
        chain_link_t* current_link = &chain->chain_links[chain->chain_links_count];
        current_link->argc = 0;

        while (token != NULL && current_link->argc < MAX_ARGS_COUNT) {
            if (strcmp(token, "|") == 0) {
                break; // End of current command
            }
            
            if (current_link->argc == 0) {
                current_link->command = strdup(token); // First token is the command name
            }

            current_link->argv[current_link->argc++] = strdup(token);
            token = strtok(NULL, " ");
        }

        // Ensure argv is null-terminated
        current_link->argv[current_link->argc] = NULL;

        chain->chain_links_count++;

        if (token != NULL) {
            token = strtok(NULL, " ");
        }
    }
}

void run_chain(chain_t* chain) {
    int prev_pipe[2];
    int curr_pipe[2];

    for (uint64_t i = 0; i < chain->chain_links_count; ++i) {
        if (pipe(curr_pipe) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(curr_pipe[0]);
            close(curr_pipe[1]);
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            if (i > 0) {
                dup2(prev_pipe[0], STDIN_FILENO); // Connect input to previous pipe
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (i < chain->chain_links_count - 1) {
                dup2(curr_pipe[1], STDOUT_FILENO); // Connect output to next pipe
                close(curr_pipe[0]);
                close(curr_pipe[1]);
            }

            execvp(chain->chain_links[i].command, chain->chain_links[i].argv);
            perror("execvp"); // If execvp fails
            exit(EXIT_FAILURE);
        } else { // Parent process
            if (i > 0) {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            prev_pipe[0] = curr_pipe[0];
            prev_pipe[1] = curr_pipe[1];
        }
    }

    close(prev_pipe[0]);
    close(prev_pipe[1]);

    for (uint64_t i = 0; i < chain->chain_links_count; ++i) {
        wait(NULL); // Wait for all child processes
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"command1 | command2 | ...\"\n", argv[0]);
        return EXIT_FAILURE;
    }

    chain_t chain;
    create_chain(argv[1], &chain);

    if (chain.chain_links_count == 0) {
        fprintf(stderr, "Error: No commands found.\n");
        return EXIT_FAILURE;
    }

    run_chain(&chain);

    // Free allocated memory
    for (uint64_t i = 0; i < chain.chain_links_count; ++i) {
        chain_link_t* link = &chain.chain_links[i];
        for (uint64_t j = 0; j < link->argc; ++j) {
            free(link->argv[j]);
        }
        free(link->command);
    }

    return EXIT_SUCCESS;
}
