#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pcre.h>

#define MAX_INDICES 30
typedef struct {
    pcre *re;
} SearchContext;

typedef struct {
    int lineNumber;
    char *content;
    size_t size;
} FileData;

pcre* compileRegex(const char *regexPattern) {
    const char *error;
    int errorOffset;
    pcre *re = pcre_compile(regexPattern, 0, &error, &errorOffset, NULL);
    if (re == NULL) {
        fprintf(stderr, "Error: Regex compilation failed at offset %d: %s\n", errorOffset, error);
        exit(1);
    }
    return re;
}

void closeFile(int fileDescriptor, FileData *fileData) {
    munmap(fileData->content, fileData->size);
    close(fileDescriptor);
}

void processLine(const char *fileName, FileData *fileData, char *lineStart, char *lineEnd) {
    if (lineEnd == NULL) {
        printf("%s:%d: %s\n", fileName, fileData->lineNumber, lineStart);
    } else {
        printf("%s:%d: %.*s\n", fileName, fileData->lineNumber, (int) (lineEnd - lineStart), lineStart);
    }
}

void search_file(const char *fileName, SearchContext *context, FileData *fileData) {
    int fileDescriptor = open(fileName, O_RDONLY);
    if (fileDescriptor == -1) {
        fprintf(stderr, "Error: Unable to open file %s\n", fileName);
        return;
    }

    struct stat fileStat;
    fstat(fileDescriptor, &fileStat);
    fileData->content = mmap(NULL, fileStat.st_size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
    fileData->size = fileStat.st_size;

    int currentOffset = 0;
    int regexMatchResult;
    int matchOffsets[MAX_INDICES];
    char *currentLineStart = fileData->content;

    while ((regexMatchResult = pcre_exec(context->re, NULL, fileData->content, fileStat.st_size, currentOffset, 0, matchOffsets, 30)) >= 0) {
        for (int i = 0; i < regexMatchResult; i++) {
            int matchStart = matchOffsets[2 * i];
            char *matchStartPtr = fileData->content + matchStart;
            char *currentLineEnd = strchr(currentLineStart, '\n');

            while (currentLineEnd != NULL && (int)(matchStartPtr - currentLineEnd) > 0) {
                currentLineStart = currentLineEnd + 1;
                ++fileData->lineNumber;
                currentLineEnd = strchr(currentLineStart, '\n');
                if (fileData->lineNumber > 25) break;
            }

            processLine(fileName, fileData, currentLineStart, currentLineEnd);
        }
        currentOffset = matchOffsets[1];
    }

    if (regexMatchResult != PCRE_ERROR_NOMATCH) {
        fprintf(stderr, "Error: Regex matching error: %d\n", regexMatchResult);
    }

    closeFile(fileDescriptor, fileData);
    fileData->lineNumber = 1;
}

void search_in_directory_recursive(const char *dirName, SearchContext *context, FileData *fileData);

void search_in_directory(const char *dirName, SearchContext *context, FileData *fileData) {
    DIR *dirp = opendir(dirName);
    if (dirp == NULL) {
        fprintf(stderr, "Error: Unable to open directory %s\n", dirName);
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dirName, dp->d_name);
            struct stat pathStat;
            stat(path, &pathStat);
            if (S_ISDIR(pathStat.st_mode)) {
                search_in_directory_recursive(path, context, fileData);
            } else {
                search_file(path, context, fileData);
            }
        }
    }
    closedir(dirp);
}

void search_in_directory_recursive(const char *dirName, SearchContext *context, FileData *fileData) {
    search_in_directory(dirName, context, fileData);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: mygrep <regex_pattern> <search_directory>\n");
        return 1;
    }

    const char *regexPattern = argv[1];
    const char *dirName = argv[2];

    SearchContext context;
    context.re = compileRegex(regexPattern);

    FileData fileData;
    fileData.lineNumber = 1;


    search_in_directory_recursive(dirName, &context, &fileData);

    pcre_free(context.re);

    return 0;
}
