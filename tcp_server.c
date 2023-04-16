#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: tcp_server <server_port> <welcome_message_file> <client_message_file>\n");
        exit(1);
    }

    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set up the server address and port
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the server address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Failed to bind socket");
        close(server_socket);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Failed to listen for connections");
        close(server_socket);
        exit(1);
    }

    // Load the welcome message from file
    char welcome_message[MAX_BUFFER_SIZE];
    FILE *welcome_file = fopen(argv[2], "r");
    if (welcome_file == NULL) {
        perror("Failed to open welcome message file");
        close(server_socket);
        exit(1);
    }
    fgets(welcome_message, MAX_BUFFER_SIZE, welcome_file);
    printf("%s", welcome_message);
    fclose(welcome_file);

    // Open client message file for writing
    FILE *out_file = fopen(argv[3], "a");
    if (out_file == NULL) {
        perror("Failed to open client message file");
        close(server_socket);
        exit(1);
    }

    // Accept incoming connections and handle them
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int client = accept(server_socket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    while (1) {
        // Receive messages from the client and write them to a file
        char buffer[MAX_BUFFER_SIZE];
        int bytes_received = recv(client, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received == -1) {
            perror("Failed to receive message from client");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected!\n");
            break;
        } else {
            printf("Message from client: %s\n", buffer);
            buffer[bytes_received] = 0;
            fprintf(out_file, "%s", buffer);
        }
    }
    // Close the client message file
    fclose(out_file);

    // Close the socket
    close(server_socket);

    return 0;
}