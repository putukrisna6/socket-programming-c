#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>

#define SIZE 50
#define STR_SIZE 1024
#define PORT 8080
#define N_OF_CONNECTION 5

pthread_t tid[3000];
int find;
int total = 0;

typedef struct akun_t {
    char name[SIZE];
    char password[SIZE];
} Account;

typedef struct file_t {
    char publisher[SIZE];
    char year[SIZE];
    char name[SIZE];
    char path[SIZE];
} Entry;

bool equal(char *s1, char *s2) {
    int i = 0, \
        d = 0;
    
    for (i = 0; s1[i] != '\0'; i++) {
        if (toupper(s1[i]) != toupper(s2[i]))
            return false;
    }
    return true;
}

void read_tsv_line(Entry *store, char *line) {
    int i = 0;
    int j = 0;
    // read file name
    while (line[i] != '/') {
        i++;
    }
    i++;
    while (line[i] != '\t') {
        store->name[j] = line[i];
        i++;
        j++;
    }
    store->name[j] = '\0';
    i++;
    j = 0;
    strcpy(store->path, "FILES/");
    strcat(store->path, store->name);

    // read publisher
    while (line[i] != '\t') {
        store->publisher[j] = line[i];
        i++;
        j++;
    }
    store->publisher[j] = '\0';
    i++;
    j = 0;

    // read year
    while (line[i] != '\n') {
        store->year[j] = line[i];
        i++;
        j++;
    }
    store->year[j] = '\0';

    return;
}

void log_action(char *type, char *fileName, char *user, char *pass) {
    FILE *log;
    char action[16];

    if (equal(type, "add")) {
        strcpy(action, "Tambah");
    }
    else if (equal(type, "delete")) {
        strcpy(action, "Hapus");
    }

    log = fopen("running.log", "a");
    fprintf(log, "%s : %s (%s:%s)\n", action, fileName, user, pass);
    fclose(log);

    return;
}

void removeLine(int line) {
    int ctr = 0;
    char ch;

    char fname[] = "files.tsv";
    char temp[] = "temp.tsv";
    FILE *fp1, *fp2;

    // buat copy ke file temp
    char str[STR_SIZE];

    fp1 = fopen(fname, "r");
    // can't open file
    if (!fp1) {
        printf("file cannot be opened\n");
        return;
    }

    fp2 = fopen(temp, "w");
    if (!fp2) {
        printf("unable to make temp\n");
        fclose(fp1);
        return;
    }

    while (!feof(fp1)) {
        strcpy(str, "\0");
        fgets(str, STR_SIZE, fp1);

        if (!feof(fp1)) {
            if (ctr != line) {
                fprintf(fp2, "%s", str);
            }
            else {
                int len = strlen(str);
                str[len-1] = '\0';
            }
            ctr++;
        }
    }
    fclose(fp1);
    fclose(fp2);
    remove(fname);
    rename(temp, fname);
}

void processPath(char *client, char *fileName) {
    int i = 0;
    int flag = 0;
    while (client[i] != '.') {
        if (client[i] == '/') {
            flag = 1;
        }
        i++;
    }
    if (flag) {
        while (client[i] != '/') {
            i--;
        }
        i++;
    }
    else {
        i = 0;
    }

    int j = 0;
    while (client[i] != '\0') {
        fileName[j] = client[i];
        i++;
        j++;
    }
    fileName[j] = '\0';

    return;
}

void *client(void *tmp) {
    FILE *fp, \
        *fp2, \
        *fp3;
    
    Account akun, akun2;

    char buffer[STR_SIZE] = {0};
    char hello[STR_SIZE] = "you're connected";
    char deny[STR_SIZE] = "someone else is using the system, please wait...";

    char *regist = "register";
    char *login = "login";
    char *quit = "quit";

    int valread;
    int new_socket = *(int *)tmp;

    if (total == 1) {
        send(new_socket, hello, STR_SIZE, 0);
    }
    else {
        send(new_socket, deny, STR_SIZE, 0);
    }

    // if (total > 1) {
    //     close(new_socket);
    //     total--;
    //     printf("total: %d\n", total);
    //     return 0;
    // }
    while (total > 1) {
        valread = read(new_socket, buffer, STR_SIZE);
        if (total == 1) {
            send(new_socket, hello, STR_SIZE, 0);
        }
        else {
            send(new_socket, deny, STR_SIZE, 0);
        }
    }

    while (true) {
        valread = read(new_socket, buffer, STR_SIZE);

        if (equal(login, buffer)) {
            valread = read(new_socket, akun.name, STR_SIZE);
            valread = read(new_socket, akun.password, STR_SIZE);

            fp3 = fopen("akun.txt", "r");

            int flag = 0;
            char *line = NULL;
            ssize_t len = 0;
            ssize_t file_read;
            while ((file_read = getline(&line, &len, fp3) != -1)) {
                char t_name[SIZE];
                char t_pass[SIZE];

                int i = 0;
                while (line[i] != ':') {
                    t_name[i] = line[i];
                    i++;
                }
                t_name[i] = '\0';
                i++;
                int j = 0;
                while (line[i] != '\n') {
                    t_pass[j] = line[i];
                    j++;
                    i++;
                }
                t_pass[j] = '\0';

                if (equal(akun.name, t_name) && equal(akun.password, t_pass)) {
                    flag = 1;
                    char *buffer = "Login success";
                    send(new_socket, buffer, strlen(buffer), 0);
                    break;
                }
            }

            if (flag == 0) {
                printf("Auth Failed\n");

                char *buffer = "Login failed";
                send(new_socket, buffer, strlen(buffer), 0);
            }
            else {
                printf("Auth success\n");

                while (true) {
                    valread = read(new_socket, buffer, STR_SIZE);
                    if (equal("logout", buffer)) {
                        break;
                    }
                    else if (equal("add", buffer)) {
                        Entry request;

                        char clientPath[STR_SIZE];

                        valread = read(new_socket, request.publisher, STR_SIZE);
                        valread = read(new_socket, request.year, STR_SIZE);
                        valread = read(new_socket, clientPath, STR_SIZE);

                        processPath(clientPath, request.name);

                        strcpy(request.path, "FILES/");
                        strcat(request.path, request.name);

                        // start adding
                        int des_fd = open(request.path, O_WRONLY | O_CREAT | O_EXCL, 0700);
                        if (!des_fd) {
                            perror("can't open file");
                            exit(EXIT_FAILURE);
                        }

                        int file_read_len;
                        char buff[STR_SIZE];

                        while (true) {
                            memset(buff, 0x00, STR_SIZE);
                            file_read_len = read(new_socket, buff, STR_SIZE);
                            write(des_fd, buff, file_read_len);
                            break;
                        }
                        // done adding

                        fp = fopen("files.tsv", "a");
                        fprintf(fp, "%s\t%s\t%s\n", request.path, request.publisher, request.year);
                        fclose(fp);

                        log_action("add", request.name, akun.name, akun.password);
                        continue;
                    }
                    else if (equal("download", buffer)) {
                        valread = read(new_socket, buffer, STR_SIZE);

                        fp = fopen("files.tsv", "r");

                        char *line = NULL;
                        ssize_t len = 0;
                        ssize_t file_read;

                        bool found = false;
                        char error_message[] = "No such file found.\n";
                        char good_message[] = "File ready to download.\n";
                        char file_loc[STR_SIZE];

                        while ((file_read = getline(&line, &len, fp) != -1)) {
                            Entry temp_entry;
                            read_tsv_line(&temp_entry, line);

                            if (equal(buffer, temp_entry.name)) {
                                found = true;
                                strcpy(file_loc, temp_entry.path);
                                break;
                            }
                        }
                        if (!found) {
                            send(new_socket, error_message, STR_SIZE, 0);
                        }
                        else {
                            send(new_socket, good_message, STR_SIZE, 0);
                            
                            printf("attempt to send: %s\n", file_loc);
                            int fd = open(file_loc, O_RDONLY);
                            if (!fd) {
                                perror("can't open");
                                exit(EXIT_FAILURE);
                            }

                            int read_len;
                            while (true) {
                                memset(file_loc, 0x00, STR_SIZE);
                                read_len = read(fd, file_loc, STR_SIZE);

                                if (read_len == 0) {
                                    break;
                                }
                                else {
                                    send(new_socket, file_loc, read_len, 0);                               
                                }
                            }
                            close(fd);
                        }
                        fclose(fp);
                    }
                    else if (equal("delete", buffer)) {
                        valread = read(new_socket, buffer, STR_SIZE);

                        fp = fopen("files.tsv", "r");

                        char *line = NULL;
                        ssize_t len = 0;
                        ssize_t file_read;

                        bool found = false;
                        char error_message[] = "No such file found.\n";
                        char good_message[] = "deleted.\n";

                        int index = 0;

                        while ((file_read = getline(&line, &len, fp) != -1)) {
                            Entry temp_entry;
                            read_tsv_line(&temp_entry, line);

                            if (equal(buffer, temp_entry.name)) {
                                found = true;
                                char old[] = "FILES/old-";
                                strcat(old, temp_entry.name);
                                rename(temp_entry.path, old);
                                log_action("delete", temp_entry.name, akun.name, akun.password);
                                break;
                            }
                            index++;
                        }
                        if (!found) {
                            send(new_socket, error_message, STR_SIZE, 0);
                        }
                        else {
                            removeLine(index);
                            send(new_socket, good_message, STR_SIZE, 0);
                        }
                        fclose(fp);
                    }
                    else if (equal("see", buffer)) {
                        fp = fopen("files.tsv", "r");
                        if (!fp) {
                            send(new_socket, "e", sizeof("e"), 0);
                            memset(buffer, 0, sizeof(buffer));
                            continue;
                        }

                        char *line = NULL;
                        ssize_t len = 0;
                        ssize_t file_read;
                        while ((file_read = getline(&line, &len, fp) != -1)) {
                            Entry temp_entry;
                            read_tsv_line(&temp_entry, line);

                            // read extension
                            char ext[SIZE];
                            int i = 0;
                            while (temp_entry.path[i] != '.') {
                                i++;
                            }
                            int j = 0;
                            while (temp_entry.path[i] != '\0') {
                                ext[j] = temp_entry.path[i];
                                i++;
                                j++;
                            }
                            ext[j] = '\0';

                            char message[STR_SIZE];
                            sprintf(message, "Nama : %s\nPublisher : %s\nTahun Publishing : %s\nEkstensi File : %s\nFilepath : %s\n\n", 
                                    temp_entry.name, temp_entry.publisher, temp_entry.year, ext, temp_entry.path);
                            
                            send(new_socket, message, STR_SIZE, 0);
                        }
                        send(new_socket, "e", sizeof("e"), 0);
                        fclose(fp);
                    }
                    else if (equal("find", buffer)) {
                        valread = read(new_socket, buffer, STR_SIZE);

                        fp = fopen("files.tsv", "r");

                        char *line = NULL;
                        ssize_t len = 0;
                        ssize_t file_read;

                        bool found = false;
                        char error_message[] = "No such file found.\n";

                        while ((file_read = getline(&line, &len, fp) != -1)) {
                            Entry temp_entry;
                            read_tsv_line(&temp_entry, line);

                            char *h;
                            if ((h = strstr(temp_entry.name, buffer)) != NULL) {
                                found = true;

                                // read extension
                                char ext[SIZE];
                                int i = 0;
                                while (temp_entry.path[i] != '.') {
                                    i++;
                                }
                                int j = 0;
                                while (temp_entry.path[i] != '\0') {
                                    ext[j] = temp_entry.path[i];
                                    i++;
                                    j++;
                                }
                                ext[j] = '\0';

                                char message[STR_SIZE];
                                sprintf(message, "Nama : %s\nPublisher : %s\nTahun Publishing : %s\nEkstensi File : %s\nFilepath : %s\n\n", 
                                        temp_entry.name, temp_entry.publisher, temp_entry.year, ext, temp_entry.path);
                                
                                send(new_socket, message, STR_SIZE, 0);
                            }
                        }
                        if (!found) {
                            send(new_socket, error_message, STR_SIZE, 0);
                        }
                        send(new_socket, "e", sizeof("e"), 0);
                        fclose(fp);
                    }
                }
            }
            fclose(fp3);
        }
        else if (equal(regist, buffer)) {
            fp2 = fopen("akun.txt", "a");

            valread = read(new_socket, akun.name, STR_SIZE);
            valread = read(new_socket, akun.password, STR_SIZE);

            fprintf(fp2, "%s:%s\n", akun.name, akun.password);
            fclose(fp2);
        }
        else if (equal(quit, buffer)) {
            close(new_socket);
            total--;
            break;
        }
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, \
        new_socket, \
        valread;
    int opt = 1;

    struct sockaddr_in address;
    int addr_len = sizeof(address);

    find = 0;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, 
        SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0 ) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, N_OF_CONNECTION) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    mkdir("FILES", 0777);

    while(true) {
        if ((new_socket = accept(server_fd, 
            (struct sockaddr *) &address, (socklen_t*) &addr_len)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_create(&(tid[total]), NULL, &client, &new_socket);
        total++;
    }

    return 0;
}
