#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *log_file;
    FILE *fp;

    if (argc != 3) {
        printf("Nhập sai tham số đầu vào!");
        return -1;
    }

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
        printf("Không thể tạo socket\n");
        return -1;
    }

    // Thiết lập địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(atoi(argv[1]));

    // Gán địa chỉ cho socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Không thể gán địa chỉ cho socket\n");
        return -1;
    }

    // Lắng nghe kết nối
    if (listen(server_fd, 5) < 0) {
        printf("Lỗi khi lắng nghe kết nối\n");
        return -1;
    }

    printf("Đang chờ client...!\n");

    // Mở file log
    log_file = argv[2];
    fp = fopen(log_file, "a");
    if (!fp) {
        printf("Không thể mở file log\n");
        return -1;
    }

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            printf("Lỗi khi chấp nhận kết nối\n");
            return -1;
        }
    // Chấp nhận kết nối và đọc dữ liệu
    while (1) {

        // Đọc dữ liệu
        valread = recv(new_socket, buffer, 1024, 0);
        // In ra màn hình và ghi vào file log
        if (valread > 0) {
            time_t now;
            struct tm *local;
            char timestamp[26];
            time(&now);
            local = localtime(&now);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local);
            printf("Dữ liệu từ client: %s %s\n", inet_ntoa(address.sin_addr), buffer);
            fprintf(fp, "%s %s %s\n", inet_ntoa(address.sin_addr), timestamp, buffer);
        } else {
            printf("Mất kết nối với client!\n");
            fclose(fp);
            break;
        }

    }
    // Đóng kết nối
    close(new_socket);

    // Đóng file log
    return 0;
}