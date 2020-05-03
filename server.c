#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 2345
#define MAX_CLIENTS 16
#define EXTENSION_TAR ".tar"
#define KEY "_!_END_PATH_!_"
#define BUF_SIZE 1024
#define UNTAR_COMMAND "tar -C "
#define UNTAR_KEYS " -xvf "
#define RM_COMMAND "rm "

void answer_to_client(int fd, char *ans) {
    write(fd, ans, strlen(ans));
}

char *itoa(int val, int base){
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}

char *generate_tar_name() {
    srand(time(NULL));
    int tar_num = rand();

    char *num;
    num = itoa(tar_num, 10);
    printf("num %s", num);

    char *prefix = "tar_tmp_file";
    char *tar_name = calloc(strlen(prefix) + strlen(num) + strlen(EXTENSION_TAR) + 1, sizeof(char));
    memcpy(tar_name, prefix, strlen(prefix));
    memcpy(tar_name + strlen(prefix), num, strlen(num));
    memcpy(tar_name + strlen(prefix) + strlen(num), EXTENSION_TAR, strlen(EXTENSION_TAR));

    return tar_name;
}

char *get_untar_command(char *path, char *tar_name) {
    printf("tut3!!!\n");        ////////
    char *untar_command = calloc(strlen(UNTAR_COMMAND) + strlen(path) + strlen(UNTAR_KEYS) + strlen(tar_name) + 1, sizeof(char));
    printf("tut3!!!\n");        ////////
    memcpy(untar_command, UNTAR_COMMAND, strlen(UNTAR_COMMAND));
    memcpy(untar_command + strlen(UNTAR_COMMAND), path, strlen(path));
    memcpy(untar_command + strlen(UNTAR_COMMAND) + strlen(path), UNTAR_KEYS, strlen(UNTAR_KEYS));
    memcpy(untar_command + strlen(UNTAR_COMMAND) + strlen(path) + strlen(UNTAR_KEYS), tar_name, strlen(tar_name));
    printf("tut3!!!\n");        ////////
    printf("%s\n", untar_command);
    return untar_command;
}

char *get_path(char *buf) {
    char *start_key = strstr(buf, KEY);
    if (start_key == NULL) {
        fprintf(stdout, "no path in data\n");
        return NULL;
    }

    size_t len = start_key - buf;
    char *path = calloc(len + 1, sizeof(char));
    if (path == NULL) {
        fprintf(stdout, "calloc error\n");
        return NULL;
    }
    char *res = memcpy(path, buf, len);
    if (res == NULL) {
        fprintf(stdout, "memcpy error\n");
        return NULL;
    }
    return path;
}

int accept_file(int fd) {
    char *buf = calloc(BUF_SIZE, sizeof(char));

    ssize_t res = read(fd, buf, BUF_SIZE);
    if (res == 0) {
        fprintf(stderr, "reading data error\n");
        answer_to_client(fd, "reading data error\n");
        free(buf);
        return -1;
    }

    char *path = get_path(buf);
    if (path == NULL) {
        fprintf(stdout, "get path error\n");
        answer_to_client(fd, "get path error\n");
        free(buf);
        return -1;
    }

    char *tar_name = generate_tar_name();

    FILE *file=fopen(tar_name, "w");
    if (file == NULL) {
        fprintf(stdout, "can not open file\n");
        answer_to_client(fd, "can not open file\n");
        free(buf);
        free(path);
        free(tar_name);
        return -1;
    }

    size_t res_write = fwrite(buf + strlen(path) + strlen(KEY), sizeof(char), res - strlen(path) - strlen(KEY), file);
    if (res - strlen(path) - strlen(KEY) != res_write) {
        fprintf(stdout, "write-end error\n");
        answer_to_client(fd, "write-end error\n");
        free(buf);
        fclose(file);
        free(path);
        free(tar_name);
        return -1;
    }

    while (res) {
        res = read(fd, buf, BUF_SIZE);
        res_write = fwrite(buf, sizeof(char), res, file);
        printf("res %d, res_w %d\n", res, res_write);
        if (res != res_write) {
            fprintf(stdout, "read-write error\n");
            answer_to_client(fd, "read-write error\n");
            free(buf);
            fclose(file);
            free(path);
            free(tar_name);
            return -1;
        }
    }
    printf("pam1\n");
    free(buf);
    fclose(file);
    char *untar_command = get_untar_command(path, tar_name);
    system(untar_command);
    free(untar_command);

    char rm_command[strlen(tar_name) + strlen(RM_COMMAND) + 1];
    memcpy(rm_command, RM_COMMAND, strlen(RM_COMMAND));
    memcpy(rm_command + strlen(RM_COMMAND), tar_name, strlen(tar_name));
    rm_command[strlen(tar_name) + strlen(RM_COMMAND)] = '\0';
    system(rm_command);

    free(tar_name);
    free(path);

    answer_to_client(fd, "accepted!");
    return 0;
}

int new_accept_file_process(int fd) {
    pid_t cpid;

    cpid = fork();
    if (cpid < 0) {
        fprintf(stderr, "fork error\n");
        return -1;
    }
    if (cpid == 0) {        //ребенок
        printf("forked\n");    //////
        accept_file(fd);
        close(fd);
        exit(EXIT_SUCCESS);
    }
    return 0;
}

int accept_client_and_take_file(int socket_fd) {
    struct sockaddr_in client_addr;
    memset((void *)&client_addr, '\0', sizeof(client_addr));
    socklen_t addr_size = sizeof(client_addr);

    int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd < 0) {
        fprintf(stderr, "accept error\n");
        return -1;
    }
    printf("accepted\n");    //////
    int res = new_accept_file_process(client_fd);
    if (client_fd < 0) {
        close(client_fd);
        return -1;
    }
    return 0;
}


int create_server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        return -1;
    }

    struct sockaddr_in srv_addr;
    memset((void *)&srv_addr, '\0', sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(SERVER_PORT);

    if (bind(fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0) {
        close(fd);
        return -1;
    }
    if (listen(fd, MAX_CLIENTS) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int main() {
    int srv_fd = create_server();
    if (srv_fd < 0){
        exit(EXIT_FAILURE);
    }

    if (accept_client_and_take_file(srv_fd) < 0) {
        close(srv_fd);
        exit(EXIT_FAILURE);
    }
    wait(NULL);
}

