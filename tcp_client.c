#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: tcp_client <server_ip_address> <server_port>\n");
        exit(1);
    }

    // Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set up the server address and port
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    if (server_address.sin_addr.s_addr == INADDR_NONE) {
        printf("Invalid IP address\n");
        close(client_socket);
        exit(1);
    }
    server_address.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Failed to connect to server");
        close(client_socket);
        exit(1);
    }

    // Receive input from the user and send it to the server
    char buffer[MAX_BUFFER_SIZE];
    while (1) {
        printf("Enter a message to send to the server (or enter 'quit' to exit): ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Send the input to the server
        int bytes_sent = 0;
        while (bytes_sent < len) {
            int ret = send(client_socket, buffer + bytes_sent, len - bytes_sent, 0);
            if (ret == -1) {
                perror("Failed to send message to server");
                close(client_socket);
                exit(1);
            }
            bytes_sent += ret;
        }

        // Check if the user wants to quit
        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }

    // Close the socket
    close(client_socket);

    return 0;
}