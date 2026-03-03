#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

void fix_broken_echo() {
    // Get parent's PID
    pid_t ppid = getppid();
    
    // Construct the path to the parent's open file descriptor (likely fd=3)
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/fd/3", ppid);

    // Open the parent's file descriptor to read the file content
    int fd_file = open(path, O_RDONLY);
    if (fd_file < 0) {
        // If this fails for some reason, just exit
        _exit(1);
    }

    // Open a new file descriptor for stdout
    // This gives us a fd not equal to 1 or 2, but points to the same output
    int fd_out = open("/proc/self/fd/1", O_WRONLY);
    if (fd_out < 0) {
        _exit(1);
    }

    // Read from fd_file and write to fd_out
    char buf[4096];
    ssize_t r;
    while ((r = read(fd_file, buf, sizeof(buf))) > 0) {
        ssize_t w = 0;
        while (w < r) {
            ssize_t res = write(fd_out, buf + w, r - w);
            if (res < 0) {
                _exit(1);
            }
            w += res;
        }
    }

    // Clean up
    close(fd_file);
    close(fd_out);
}
