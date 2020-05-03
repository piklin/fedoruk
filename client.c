#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 2345
#define KEY "_!_END_PATH_!_"
#define BUF_SIZE 128
#define TAR_COMMAND "tar -cvf - "

char *itoa(int val, int base){
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}

char *get_path_with_key(char *path) {
    char *path_key = malloc(BUF_SIZE * sizeof(char));
    memcpy(path_key, path, strlen(path));
    memcpy(path_key + strlen(path), KEY, strlen(KEY));
    memset(path_key + strlen(path) + strlen(KEY), '-', BUF_SIZE - strlen(path) - strlen(KEY));
    return path_key;
}

char *get_tar_command(char *path, int fd) {

    char *sym = " >&";
    char *fd_str = itoa(fd, 10);
    char *tar_command = calloc(strlen(TAR_COMMAND) + strlen(path) + strlen(sym) + strlen(fd_str) + 1, sizeof(char));

    memcpy(tar_command, TAR_COMMAND, strlen(TAR_COMMAND));
    memcpy(tar_command + strlen(TAR_COMMAND), path, strlen(path));
    memcpy(tar_command + strlen(TAR_COMMAND) + strlen(path), sym, strlen(sym));
    memcpy(tar_command + strlen(TAR_COMMAND) + strlen(path) + strlen(sym), fd_str, strlen(fd_str));

    return tar_command;
}

char *get_ip_address(char *str) {
    char *pos = strchr(str, '@');
    if (pos == NULL) {
        return NULL;
    }
    char *ip = calloc(pos - str + 1, sizeof(char));
    memcpy(ip, str, pos - str);
    return ip;
}

char *get_dst_path(char *str) {
    char *pos = strchr(str, '@');
    if (pos == NULL) {
        return NULL;
    }

    size_t len = strlen(str) - (pos - str) - 1;
    char *path = calloc(len + 1, sizeof(char));
    memcpy(path, pos + 1, len);
    return path;
}

int connect_to_srv(char *ip_addr_str) {

    struct sockaddr_in addr;
    memset((void *)&addr, '\0', sizeof(addr));

    if (inet_aton(ip_addr_str, &addr.sin_addr) < 0) {
        printf("Invalid ip address\n");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(socket_fd);
        printf("Connecting error\n");
        return -1;
    }
    return socket_fd;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Invalid args format\n");
        exit(EXIT_FAILURE);
    }

    char *ip = get_ip_address(argv[2]);
    char *path_dst = get_dst_path(argv[2]);
    if (ip == NULL || path_dst == NULL) {
        return -1;
    }

    int fd = connect_to_srv(ip);
    if (fd < 0) {
        free(ip);
        free(path_dst);
        return -1;
    }

    char *buf = get_path_with_key(path_dst);
    ssize_t res = write(fd, buf, strlen(buf));
    if (res != strlen(buf)) {
        printf("Error writing to server!\n");
        close(fd);
        return -1;
    }

    char *tar_command = get_tar_command(argv[1], fd);
    system(tar_command);

    printf("Sent!\n");
    close(fd);
    return 0;
}

