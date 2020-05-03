#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_PORT 2345
#define MAX_CLIENTS 16
#define KEY "_!_END_PATH_!_"
#define BUF_SIZE 1024
#define UNTAR_COMMAND "tar -xf - -C "

pid_t *children;

void stop(int sig){
    for(size_t i = 0; i < MAX_CLIENTS; i++) {
        if (children[i] != 0) {
            kill(children[i], SIGTERM);
            wait(NULL);
        }
    }
    exit(0);
}

void set_pid(pid_t pid) {
    for(size_t i = 0; i < MAX_CLIENTS; i++) {
        if (children[i] == 0) {
            children[i] = pid;
            return;
        }
    }
}

void del_pid(pid_t pid) {
    for(size_t i = 0; i < MAX_CLIENTS; i++) {
        if (children[i] == pid) {
            children[i] = 0;
            return;
        }
    }
}

char *itoa(int val, int base){
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}

char *get_untar_command(char *path, int fd) {

    char *sym = " <&";
    char *fd_str = itoa(fd, 10);

    char *untar_command = calloc(strlen(UNTAR_COMMAND) + strlen(path) + strlen(sym) + strlen(fd_str) + 1, sizeof(char));

    memcpy(untar_command, UNTAR_COMMAND, strlen(UNTAR_COMMAND));
    memcpy(untar_command + strlen(UNTAR_COMMAND), path, strlen(path));
    memcpy(untar_command + strlen(UNTAR_COMMAND) + strlen(path), sym, strlen(sym));
    memcpy(untar_command + strlen(UNTAR_COMMAND) + strlen(path) + strlen(sym), fd_str, strlen(fd_str));

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
        free(buf);
        return -1;
    }

    char *path = get_path(buf);
    if (path == NULL) {
        fprintf(stderr, "get path error\n");
        free(buf);
        return -1;
    }

    char *untar_command = get_untar_command(path, fd);
    system(untar_command);

    free(buf);
    free(untar_command);
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
        accept_file(fd);
        close(fd);
        del_pid(getpid());
        exit(EXIT_SUCCESS);
    } else {
        set_pid(cpid);
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

    children = calloc(MAX_CLIENTS, sizeof(int));
    signal(SIGTERM, stop);
    printf("%d", getpid());

    while (1) {
        if (accept_client_and_take_file(srv_fd) < 0) {
            close(srv_fd);
            exit(EXIT_FAILURE);
        }
    }
}
