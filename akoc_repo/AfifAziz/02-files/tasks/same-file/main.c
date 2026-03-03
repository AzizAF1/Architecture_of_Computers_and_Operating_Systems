#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

bool is_same_file(const char* lhs_path, const char* rhs_path) {
    struct stat left_file;
    struct stat right_file;
    if (stat(lhs_path, &left_file) == -1) {
        return false;
    }

    if (stat(rhs_path, &right_file) == -1) {
        return false;
    }

    return (left_file.st_ino == right_file.st_ino && left_file.st_dev == right_file.st_dev);
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        return -1;
    }
    if (is_same_file(argv[1], argv[2])) {
      printf("yes");
    } else {
      printf("no");
    }
    return 0;
}
