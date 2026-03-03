#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>

// A simple key-value store implemented as a singly linked list:
typedef struct kv_node {
    char *key;
    char *value;
    struct kv_node *next;
} kv_node;

static kv_node *kv_store = NULL;

// Function to trim whitespace at the end of a string
static void rtrim(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

// Find value by key
static char* kv_get(const char *key) {
    kv_node *cur = kv_store;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            return cur->value;
        }
        cur = cur->next;
    }
    return NULL; // not found
}

// Set (insert or update) key-value
static void kv_set(const char *key, const char *value) {
    kv_node *cur = kv_store;
    // Try to find existing key
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            free(cur->value);
            cur->value = strdup(value);
            return;
        }
        cur = cur->next;
    }
    // Not found, insert new
    kv_node *new_node = (kv_node*)malloc(sizeof(kv_node));
    new_node->key = strdup(key);
    new_node->value = strdup(value);
    new_node->next = kv_store;
    kv_store = new_node;
}

#define MAX_CLIENTS  FD_SETSIZE
#define BUFFER_SIZE  4096

typedef struct {
    int fd;
    char buffer[BUFFER_SIZE];
    int buf_used;
} client_state;

static client_state clients[MAX_CLIENTS];

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Invalid port: %d\n", port);
        return 1;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 128) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].buf_used = 0;
    }

    fd_set readfds;
    int maxfd = listen_fd;

    fprintf(stderr, "Server listening on 127.0.0.1:%d\n", port);

    for (;;) {
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        maxfd = listen_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd)
                    maxfd = clients[i].fd;
            }
        }

        int ret = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // New connection
        if (FD_ISSET(listen_fd, &readfds)) {
            int newfd = accept(listen_fd, NULL, NULL);
            if (newfd < 0) {
                perror("accept");
                continue;
            }

            int slot = -1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd < 0) {
                    slot = i;
                    break;
                }
            }
            if (slot < 0) {
                close(newfd);
            } else {
                clients[slot].fd = newfd;
                clients[slot].buf_used = 0;
            }
        }

        // Handle existing clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd < 0) continue;
            if (FD_ISSET(clients[i].fd, &readfds)) {
                int n = read(clients[i].fd, clients[i].buffer + clients[i].buf_used,
                             BUFFER_SIZE - clients[i].buf_used - 1);
                if (n < 0) {
                    perror("read");
                    close(clients[i].fd);
                    clients[i].fd = -1;
                } else if (n == 0) {
                    close(clients[i].fd);
                    clients[i].fd = -1;
                } else {
                    clients[i].buf_used += n;
                    clients[i].buffer[clients[i].buf_used] = '\0';

                    // Try to find newline
                    char *start = clients[i].buffer;
                    char *newline = strchr(start, '\n');

                    // If we have a newline, process line-based command
                    if (newline) {
                        *newline = '\0';
                        char line[BUFFER_SIZE];
                        strncpy(line, start, sizeof(line)-1);
                        line[sizeof(line)-1] = '\0';
                        rtrim(line);

                        // Move leftover data
                        int leftover = clients[i].buf_used - (newline + 1 - clients[i].buffer);
                        if (leftover > 0)
                            memmove(clients[i].buffer, newline+1, leftover);
                        clients[i].buf_used = leftover;

                        // Parse and handle command
                        char *cmd = strtok(line, " ");
                        if (cmd) {
                            if (strcmp(cmd, "get") == 0) {
                                char *key = strtok(NULL, " ");
                                if (!key) {
                                    const char *resp = "\n";
                                    write(clients[i].fd, resp, strlen(resp));
                                } else {
                                    char *val = kv_get(key);
                                    if (!val) val = "";
                                    write(clients[i].fd, val, strlen(val));
                                    write(clients[i].fd, "\n", 1);
                                }
                            } else if (strcmp(cmd, "set") == 0) {
                                char *key = strtok(NULL, " ");
                                if (key) {
                                    char *value = strtok(NULL, "");
                                    if (!value) value = "";
                                    kv_set(key, value);
                                }
                            }
                        }

                    } else {
                        // No newline found, try to parse the whole buffer as one command
                        // This handles the test scenario where no newline is sent.
                        if (clients[i].buf_used > 0) {
                            char line[BUFFER_SIZE];
                            strncpy(line, clients[i].buffer, clients[i].buf_used);
                            line[clients[i].buf_used] = '\0';
                            rtrim(line);

                            char *cmd = strtok(line, " ");
                            if (cmd) {
                                if (strcmp(cmd, "get") == 0) {
                                    char *key = strtok(NULL, " ");
                                    if (!key) {
                                        const char *resp = "\n";
                                        write(clients[i].fd, resp, strlen(resp));
                                    } else {
                                        char *val = kv_get(key);
                                        if (!val) val = "";
                                        write(clients[i].fd, val, strlen(val));
                                        write(clients[i].fd, "\n", 1);
                                    }
                                    clients[i].buf_used = 0;
                                } else if (strcmp(cmd, "set") == 0) {
                                    char *key = strtok(NULL, " ");
                                    if (key) {
                                        char *value = strtok(NULL, "");
                                        if (!value) value = "";
                                        kv_set(key, value);
                                    }
                                    clients[i].buf_used = 0;
                                }
                                // Unknown or incomplete command? Just clear buffer
                                else {
                                    clients[i].buf_used = 0;
                                }
                            } else {
                                // Empty line or no valid command
                                clients[i].buf_used = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}
