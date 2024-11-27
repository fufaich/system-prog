#include "../utils/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void process_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    while (1) {
        // Получаем запрос от клиента
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) {
            break; // Клиент закрыл соединение
        }
        buffer[bytes_read] = '\0';

        // Обработка команды
        char command[10];
        int key;
        char value[30];
        sscanf(buffer, "%s %d %[^\n]", command, &key, value);

        if (strcmp(command, "add") == 0) {
            if (add(key, value) == 0) {
                send(client_socket, "Key added successfully\n", 24, 0);
            } else {
                send(client_socket, "Failed to add key\n", 19, 0);
            }
        } else if (strcmp(command, "delete") == 0) {
            if (delete(key) == 0) {
                send(client_socket, "Key deleted successfully\n", 26, 0);
            } else {
                send(client_socket, "Failed to delete key\n", 22, 0);
            }
        } else if (strcmp(command, "search") == 0) {
            const char* result = search(key);
            if (result) {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "Found: %s\n", result);
                send(client_socket, response, strlen(response), 0);
            } else {
                send(client_socket, "Key not found\n", 15, 0);
            }
        } else if (strcmp(command, "exit") == 0) {
            send(client_socket, "Goodbye!\n", 9, 0);
            break;
        } else {
            send(client_socket, "Unknown command\n", 17, 0);
        }
    }
    close(client_socket);
}

int main() {
    if (createdb() != 0) {
        perror("Failed to initialize database");
        return EXIT_FAILURE;
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Создаем сокет
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязываем сокет к порту
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Ожидаем подключения
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("New client connected\n");

        // Обработка запроса клиента
        process_request(client_socket);
    }

    close(server_socket);
    return 0;
}