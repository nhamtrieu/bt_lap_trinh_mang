#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: udp_file_sender <receiver_ip> <receiver_port> <file_path>\n");
        return 1;
    }

    // Open the input file
    FILE *input_file = fopen(argv[3], "rb");
    if (input_file == NULL) {
        printf("Error: Cannot open file '%s'.\n", argv[3]);
        return 1;
    }

    // Create the UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Set up the receiver address
    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(atoi(argv[2]));
    receiver_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Read the input file and send it to the receiver
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, input_file)) > 0) {
        sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
    }

    // Send an empty packet to signal the end of the file
    sendto(sockfd, "", 0, 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

    // Clean up
    fclose(input_file);
    close(sockfd);

    return 0;
}