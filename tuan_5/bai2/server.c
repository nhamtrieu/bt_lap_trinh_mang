#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX_BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER_SIZE] = {0};
    int count = 0;

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
        perror("Không thể tạo socket");
        exit(EXIT_FAILURE);
    }

    // Cài đặt thông tin địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Liên kết socket với địa chỉ địa phương
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Không thể liên kết socket");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_fd, 3) < 0) {
        perror("Không thể lắng nghe kết nối");
        exit(EXIT_FAILURE);
    }

    printf("Đang chờ kết nối từ client...\n");

    // Chấp nhận kết nối từ client
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
        perror("Không thể chấp nhận kết nối");
        exit(EXIT_FAILURE);
    }

    // Nhận dữ liệu từ client
    int total_bytes_received = 0;
    while (total_bytes_received < MAX_BUFFER_SIZE) {
        int bytes_received = recv(new_socket, buffer + total_bytes_received, MAX_BUFFER_SIZE - total_bytes_received, 0);
        if (bytes_received <= 0) {
            break;
        }
        total_bytes_received += bytes_received;
    }

    // Xử lý dữ liệu
    printf("Đã nhận được %d byte dữ liệu từ client\n", total_bytes_received);
    char *p = buffer;
    while ((p = strstr(p, "0123456789")) != NULL) {
        count++;
        p += strlen("0123456789");
    }
    printf("Số lần xuất hiện của xâu '0123456789' là: %d\n", count);

    return 0;
}