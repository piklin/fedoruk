#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SRV_PORT 12345
#define BUF_LEN 4
#define READ_BUF_LEN 16

void flush_stdin() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { }     //чистим stdin
}

int read_shot(char *buf) {
    char *read_buf = malloc(READ_BUF_LEN);
    size_t len = READ_BUF_LEN;

    printf("стреляю: ");
    fflush(stdout);
    while(1) {
        ssize_t res = getline(&read_buf, &len, stdin);
        if (res == 3 && read_buf[0] >= 'a' && read_buf[0] <= 'h' && read_buf[1] >= '1' && read_buf[1] <= '8') {
            buf[1] = read_buf[0];
            buf[2] = read_buf[1];
            free(read_buf);
            return 0;
        }
        printf("неверный формат! Повторите ввод\nстреляю: ");
        fflush(stdout);
    }
}

int read_move(char *buf) {
    char read_buf[BUF_LEN];
    size_t len = BUF_LEN;

    while(1) {
        ssize_t res = getline(&buf, &len, stdin);
        if (!strcmp(buf, "мимо\n")) {
            buf[0] = 'M';
            return 0;
        } else if (!strcmp(buf, "ранил\n")) {
            buf[0] = 'R';
            return 1;
        } else if (!strcmp(buf, "убил\n")) {
            buf[0] = 'K';
            return 1;
        } else if (!strcmp(buf, "stop\n")) {
            buf[0] = 'S';
            return 2;
        } else {
            printf("неверный формат! Повторите ввод\nЯ: ");
            fflush(stdout);
        }
    }
}

int move(int fd, char *buf) {
    printf("Противник: ");
    fflush(stdout);
    ssize_t res = read(fd, buf, BUF_LEN);
    if (res == 0) {                                     //проверка данных
        printf("\nОшибка передечи данных!\n");
        return -1;
    }

    if (buf[0] == 'M' || buf[0] == 'N') {
        if (buf[0] == 'M') {
            printf("мимо, ");
        }
        printf("cтреляю %c%c.\nЯ: ", buf[1], buf[2]);
        fflush(stdout);
        memset(buf, '\0', BUF_LEN);

        res = read_move(buf);
        if (res == 0) {
            read_shot(buf);
        } else if (res == 1) {
            buf[1] = 'N';
            buf[2] = 'N';
        } else {
            write(fd, buf, BUF_LEN);
            printf("Игра окончена! Вы проиграли!\n");
            return 1;
        }
    } else {
        if (buf[0] == 'S') {
            printf("Игра окончена! Вы победили!\n");
            return 1;
        }
        if (buf[0] == 'R') {
            printf("ранил.\n");
        } else if (buf[0] == 'K') {
            printf("убил.\n");
        }
        memset(buf, '\0', BUF_LEN);

        printf("Я: ");
        fflush(stdout);
        buf[0] = 'N';
        read_shot(buf);
    }
    write(fd, buf, BUF_LEN);                        //отправляем ход противнику
    memset(buf, '\0', BUF_LEN);                             //чистим буфер
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
        if (move(client_fd, buf) != 0) {
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
    buf[0] = 'N';
    printf("Я: ");
    fflush(stdout);
    read_shot(buf);
    write(socket_fd, buf, BUF_LEN);                 //отправляем ход противнику
    memset(buf, '\0', BUF_LEN);
    while(1) {                                      //цикл игры
        if (move(socket_fd, buf) != 0) {
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

