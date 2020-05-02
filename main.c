#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SRV_PORT 12345
#define BUF_LEN 16

void flush_stdin() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { }     //чистим stdin
}

int read_move(int fd, char *buf) {
    printf("ход противника: ");
    fflush(stdout);
    int res = read(fd, buf, BUF_LEN);                   //читаем ход противника
    if (res == 0 || !strcmp(buf, "stop\n")) {           //проверка данных
        printf("\nигра окончена\n");
        return -1;
    }
    printf("%s", buf);                                  //выводим ход противника
    memset(buf, '\0', res);                              //чистим буфер
    return 0;
}

int write_move(int fd, char *buf) {                     //чтение
    printf("мой ход: ");
    fflush(stdout);                                     //"проталкиваем" stdout
    size_t len = BUF_LEN;
    int res = getline(&buf, &len, stdin);               //считываем ход
    if (!strcmp(buf, "stop\n") || res == 0) {           //проверяем ход на корректность
        write(fd, "stop\n", strlen("stop\n"));
        printf("\nигра окончена\n");
        return -1;
    }
    write(fd, buf, strlen(buf));                        //отправляем ход противнику
    memset(buf, '\0', res);                             //чистим буфер
    return 0;
}

int start_server(){                                     //созданиие сервера
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }

    struct sockaddr_in srv_addr;
    memset((void *)&srv_addr, '\0', sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(SRV_PORT);

    if (bind(socket_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0) {
        close(socket_fd);
        return -1;
    }
    if (listen(socket_fd, 1) < 0) {
        close(socket_fd);
        return -1;
    }
    return socket_fd;
}

int accept_client(int socket_fd) {                  //принимаем клиента
    struct sockaddr_in client_addr;
    memset((void *)&client_addr, '\0', sizeof(client_addr));
    socklen_t addr_size = sizeof(client_addr);

    printf("Ждем соперника\n");
    int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd < 0) {
        printf("Ошибка подключения\n");
        return -1;
    }
    char *result = inet_ntoa(client_addr.sin_addr);
    printf("%s connected\n", result);
    return client_fd;
}

int server() {                              //функция игры как сервер
    int socket_fd = start_server();
    if (socket_fd < 0) {
        return -1;
    }

    int client_fd = accept_client(socket_fd);
    if (client_fd < 0) {
        return -1;
    }

    char *buf = calloc(BUF_LEN, sizeof(char));
    //flush_stdin();
    while(1) {                                      //цикл игры
        if (read_move(client_fd, buf) < 0) {
            break;
        }
        if (write_move(client_fd, buf) < 0) {
            break;
        }
    }
    free(buf);
    close(client_fd);
    close(socket_fd);
    return 0;
}

int client() {                          //функция игры как клиент
    printf("Введите ip адрес\n");
    char ip_addr_str[16];
    fgets(ip_addr_str, 16, stdin);

    printf("Введите порт\n");
    int port = 0;
    scanf("%d", &port);

    struct sockaddr_in addr;
    memset((void *)&addr, '\0', sizeof(addr));
    if (inet_aton(ip_addr_str, &addr.sin_addr) < 0) {       //преобразуем адрес
        printf("некорректный адрес\n");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);        //преобразуем порт

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        close(socket_fd);
        printf("ошибка подключения\n");
        return -1;
    }

    char *buf = calloc(BUF_LEN, sizeof(char));
    flush_stdin();
    while(1) {
        if (write_move(socket_fd, buf) < 0) {       //получаем ход противника
            break;
        }
        if (read_move(socket_fd, buf) < 0) {        //отправляем свой ход
            break;
        }
    }
    free(buf);                  //чистка ресурсов
    close(socket_fd);
    return 0;
}

int main(){
    while (1) {
        printf("Создать новую игру или подключиться к сопернику?\n");
        printf("Введите new или connect\n");
        char command[9];
        fgets(command, 9, stdin);              //принимаем опцию игры
        if (!strcmp("new\n", command)) {
            server();                       //запуск игры как сервер
            break;
        } else if(!strcmp("connect\n", command)) {
            client();                       //запуск игры как клиент
            break;
        }
    }
    return 0;
}

