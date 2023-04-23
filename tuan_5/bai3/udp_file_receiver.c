#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: udp_file_receiver <receiver_port> <output_file>\n");
        return 1;
    }

    // Open the output file
    FILE *output_file = fopen(argv[2], "wb");
    if (output_file == NULL) {
        printf("Error: Cannot create file '%s'.\n", argv[2]);
        return 1;
    }

    // Create the UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Bind to the specified port
    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(atoi(argv[1]));
    receiver_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Receive data from the sender and write it to the output file
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL)) > 0) {
        fwrite(buffer, 1, bytes_received, output_file);
        if (bytes_received < BUFFER_SIZE) {
            break;
        }
    }

    // Clean up
    fclose(output_file);
    close(sockfd);

    return 0;
}