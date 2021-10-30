#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <termios.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>

#define PORT 8080
#define STR_SIZE 1024

int sock = 0, valread;
int status = 0;
pthread_t thread;

static struct termios stored_settings;

bool equal(char *s1, char *s2) {
    int i = 0, \
        d = 0;
    
    for (i = 0; s1[i] != '\0'; i++) {
        if (toupper(s1[i]) != toupper(s2[i]))
            return false;
    }
    return true;
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address, \
                        serv_addr;
    char *login = "login";
    char *regist = "register";
    char *logout = "logout";
    char *quit = "quit";
    char command[STR_SIZE], username[STR_SIZE], password[STR_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("failed to create socket\n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    char temp;
    char buffer[STR_SIZE] = {0};

    valread = read(sock, buffer, STR_SIZE);
    printf("%s\n", buffer);

    char hello[STR_SIZE] = "you're connected";
    while (!equal(hello, buffer)) {
        printf("You can periodically type: 'Check' to check whether you can use the system\n");
        scanf("%s", command);
        send(sock, command, STR_SIZE, 0);
        valread = read(sock, buffer, STR_SIZE);
        printf("%s\n", buffer);
    }

    if (!equal(hello, buffer)) {
        return 0;
    }

    memset(buffer, 0, sizeof(buffer));

    while (true) {
        printf("1. Login\n2. Register\n3. Quit\n");
        scanf("%s", command);

        if (equal(command, login)) {
            send(sock, login, strlen(login), 0);
            
            printf("Username: ");
            scanf("%c", &temp);
            scanf("%[^\n]", username);
            send(sock, username, STR_SIZE, 0);

            printf("Password: ");
            scanf("%c", &temp);
            scanf("%[^\n]", password);
            send(sock, password, STR_SIZE, 0);

            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, STR_SIZE);
            printf("%s\n", buffer);

            if (equal(buffer, "Login success")) {
                while (true) {
                    printf("1. Logout\n2. Add\n3. Download [File]\n4. Delete [File]\n5. See\n6. Find [File]\n");
                    scanf("%s", command);

                    if (equal(command, logout)) {
                        send(sock, command, STR_SIZE, 0);
                        break;
                    }
                    else if (equal(command, "add")) {
                        send(sock, command, STR_SIZE, 0);
                        
                        char data[STR_SIZE];

                        printf("Publisher: ");
                        scanf("%c", &temp);
                        scanf("%[^\n]", data);
                        send(sock, data, STR_SIZE, 0);

                        printf("Tahun Publikasi: ");
                        scanf("%c", &temp);
                        scanf("%[^\n]", data);
                        send(sock, data, STR_SIZE, 0);

                        printf("Filepath: ");
                        scanf("%c", &temp);
                        scanf("%[^\n]", data);
                        send(sock, data, STR_SIZE, 0);

                        int fd = open(data, O_RDONLY);
                        if (!fd) {
                            perror("can't open");
                            exit(EXIT_FAILURE);
                        }

                        int read_len;
                        while (true) {
                            memset(data, 0x00, STR_SIZE);
                            read_len = read(fd, data, STR_SIZE);

                            if (read_len == 0) {
                                break;
                            }
                            else {
                                send(sock, data, read_len, 0);                               
                            }
                        }
                        close(fd);
                        continue;
                    }
                    else if (equal(command, "download")) {
                        send(sock, command, STR_SIZE, 0);
                        memset(buffer, 0, sizeof(buffer));

                        scanf("%s", command);
                        send(sock, command, STR_SIZE, 0);
                        valread = read(sock, buffer, STR_SIZE);

                        char good_message[] = "File ready to download.\n";
                        if (equal(buffer, good_message)) {
                            printf("attempting to download: %s\n", command);
                            int des_fd = open(command, O_WRONLY | O_CREAT | O_EXCL, 0700);
                            if (!des_fd) {
                                perror("can't open file");
                                exit(EXIT_FAILURE);
                            }

                            int file_read_len;
                            char buff[STR_SIZE];

                            while (true) {
                                memset(buff, 0x00, STR_SIZE);
                                file_read_len = read(sock, buff, STR_SIZE);
                                write(des_fd, buff, file_read_len);
                                break;
                            }
                        }

                        printf("message: %s", buffer);
                        continue;
                    }
                    else if (equal(command, "delete")) {
                        send(sock, command, STR_SIZE, 0);
                        memset(buffer, 0, sizeof(buffer));

                        scanf("%s", command);
                        send(sock, command, STR_SIZE, 0);
                        valread = read(sock, buffer, STR_SIZE);

                        printf("message: %s", buffer);
                    }
                    else if (equal(command, "see")) {
                        send(sock, command, STR_SIZE, 0);
                        memset(buffer, 0, sizeof(buffer));

                        while (valread = read(sock, buffer, STR_SIZE)) {
                            if (equal(buffer, "e")) {
                                break;
                            }
                            printf("%s", buffer);
                        }

                        continue; 
                    }
                    else if (equal(command, "find")) {
                        send(sock, command, STR_SIZE, 0);
                        memset(buffer, 0, sizeof(buffer));

                        scanf("%s", command);
                        send(sock, command, STR_SIZE, 0);

                        while (valread = read(sock, buffer, STR_SIZE)) {
                            if (equal(buffer, "e")) {
                                break;
                            }
                            printf("%s", buffer);
                        }
                        continue;
                    }
                    else {
                        printf("command not recognized\n");
                        continue;
                    }
                }
                memset(buffer, 0, sizeof(buffer));
            }

            memset(buffer, 0, sizeof(buffer));
        }
        else if (equal(command, regist)) {
            send(sock, regist, strlen(regist), 0);
            printf("Register\n");
            printf("Username: ");
            scanf("%c", &temp);
            scanf("%[^\n]", username);
            send(sock, username, STR_SIZE, 0);

            printf("Password: ");
            scanf("%c", &temp);
            scanf("%[^\n]", password);
            send(sock, password, STR_SIZE, 0);

            printf("register success\n");
        }
        else if (equal(command, quit)) {
            send(sock, quit, strlen(quit), 0);
            printf("closing\n");
            return 0;
        }
        else {
            printf("command not recognized\n");
            continue;
        }
    }
    
    return 0;
}
