#define _GNU_SOURCE
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int signal_pipe[2];

void signal_handler(int signo) {
    write(signal_pipe[1], &signo, sizeof(int));
}

int signalfd(void) {
    if (pipe(signal_pipe) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    for (int sig_num = 1; sig_num < 32; ++sig_num) {
        if ((sig_num != SIGKILL) && (sig_num != SIGSTOP)) {
            signal(sig_num, signal_handler);
        }
    }

    return signal_pipe[0];
}
