#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>


#define BUFFER_SIZE 4096
int main(int argc, const char* argv[]){
    if(argc != 2) {
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if( fd == -1) {
        close(fd);
        return -1;
    }
    char buffer[BUFFER_SIZE];
    ssize_t size_read;
    while(1) {
        size_read = read(fd, buffer, BUFFER_SIZE);
        if(size_read == -1) {
            close(fd);
            return -1;
        }
        else if(size_read == 0) {
            usleep(1);
            continue;
        }
        ssize_t size_write = write(STDOUT_FILENO, buffer, size_read);
        if (size_write == -1) {
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}

