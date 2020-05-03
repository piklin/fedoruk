#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 2345
#define TEMP_TAR_NAME "tmp_cprem.tar"
#define KEY "_!_END_PATH_!_"
#define BUF_SIZE 5000
#define TAR_COMMAND "tar -cvf "
#define RM_COMMAND "rm "

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
        printf("неправильный адрес\n");
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
        printf("ошибка при подключении\n");
        return -1;
    }
    return socket_fd;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Неверный формат аргументов\n");
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

    size_t len = strlen(argv[1]) + strlen(TEMP_TAR_NAME) + strlen(TAR_COMMAND);
    char tar_command[len + 2];
    memcpy(tar_command, TAR_COMMAND, strlen(TAR_COMMAND));
    memcpy(tar_command + strlen(TAR_COMMAND), TEMP_TAR_NAME, strlen(TEMP_TAR_NAME));
    tar_command[strlen(TAR_COMMAND) + strlen(TEMP_TAR_NAME)] = ' ';
    memcpy(tar_command + strlen(TAR_COMMAND) + strlen(TEMP_TAR_NAME) + 1, argv[1], strlen(argv[1]));
    tar_command[len + 1] = '\0';
    printf("%s", tar_command);

    char rm_command[strlen(TEMP_TAR_NAME) + strlen(RM_COMMAND) + 1];
    memcpy(rm_command, RM_COMMAND, strlen(RM_COMMAND));
    memcpy(rm_command + strlen(RM_COMMAND), TEMP_TAR_NAME, strlen(TEMP_TAR_NAME));
    rm_command[strlen(TEMP_TAR_NAME) + strlen(RM_COMMAND)] = '\0';

    system(tar_command);

    FILE *file=fopen(TEMP_TAR_NAME, "r");
    if (file == NULL) {
        printf("fileread\n");
        close(fd);
        free(ip);
        free(path_dst);
        return -1;
    }

    char *buf = calloc(BUF_SIZE, sizeof(char));
    memcpy(buf, path_dst, strlen(path_dst));
    memcpy(buf + strlen(path_dst), KEY, strlen(KEY));
    ssize_t res = write(fd, buf, strlen(KEY) + strlen(path_dst));
    if (res != strlen(KEY) + strlen(path_dst)) {
        free(buf);
        printf("write path\n");
        fclose(file);
        close(fd);
        free(ip);
        free(path_dst);
        return -1;
    }

    int sum = 0;
    size_t res_read = 0;
    while (res) {
        res_read = fread(buf, sizeof(char), BUF_SIZE, file);
        res = write(fd, buf, res_read);
        printf("res_read %d, res %d\n", res_read, res);
        sum += res;
        if (res != res_read) {
            fclose(file);
            close(fd);
            free(buf);
            free(ip);
            free(path_dst);
            return -1;
        }
    }

    printf("sum %d\n", sum);
    fclose(file);
    free(ip);
    free(path_dst);

    system(rm_command);

    res = read(fd, buf, BUF_SIZE);
    if (res != 0) {
        printf("Result: %s\n", buf);
    }
    close(fd);

    return 0;
}

