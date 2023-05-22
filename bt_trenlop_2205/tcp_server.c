#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#define MAX_CLIENTS 20
#define BUFFER_SIZE 1024

void remove_spaces(char *str) {
    char *src = str;
    char *dst = str;

    // Xóa khoảng trắng ở đầu xâu
    while (*src && *src == ' ') {
        src++;
    }

    // Duyệt qua xâu và xóa khoảng trắng ở giữa các từ
    while (*src) {
        if (*src == ' ') {
            // Xóa khoảng trắng ở giữa các từ
            if (dst > str && *(dst-1) != ' ') {
                *(dst++) = *(src++);
            } else {
                src++;
            }
        } else {
            *(dst++) = *(src++);
        }
    }

    // Xóa khoảng trắng ở cuối xâu
    if (dst > str && *(dst-1) == ' ') {
        dst--;
    }

    *dst = '\0';
    // Viet hoa ky tu dau
    char *p = str;

    while (*p) {
        if (p == str) {
            *p = toupper(*p);
        } else {
            *p = tolower(*p);
        }
        p++;
    }
}
int main(int argc, char *argv[]) {
    int count_client = 0;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);

    // Khởi tạo socket và gán địa chỉ IP và cổng kết nối cho socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Cho phép socket lắng nghe kết nối từ các client
    listen(server_socket, MAX_CLIENTS);

    // Tạo một tập hợp các socket để sử dụng trong hàm select
    fd_set fds;
    int max_sd, activity, sd;
    int client_sockets[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        // Xóa tập hợp fds và thêm socket server vào tập hợp        
        FD_ZERO(&fds);
        FD_SET(server_socket, &fds);
        max_sd = server_socket;

        // Thêm các socket client vào tập hợp fds
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &fds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Sử dụng hàm select để kiểm tra xem liệu có kết nối mới đến hay không
        activity = select(max_sd + 1, &fds, NULL, NULL, NULL);

        // Nếu có kết nối mới đến, sử dụng hàm accept để chấp nhận kết nối mới từ client
        if (FD_ISSET(server_socket, &fds)) {
            int client_socket = accept(server_socket, (struct sockaddr *)NULL, NULL);
            count_client ++;

            // Thêm socket mới vào tập hợp các socket client
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        // Xử lý các kết nối từ client
        for (int i= 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];

            // Nếu socket client có hoạt động
            if (FD_ISSET(sd, &fds)) {
                // Nhận dữ liệu từ client
                memset(buffer, 0, sizeof(buffer));
                int valread = recv(sd, buffer, sizeof(buffer), 0);

                // Nếu client gửi chuỗi "exit", server sẽ gửi chuỗi "Tạm biệt!" và đóng kết nối
                if (strncmp(buffer, "exit", 4) == 0) {
                    send(sd, "Tạm biệt!", strlen("Tạm biệt!"), 0);
                    close(sd);
                    client_sockets[i] = 0;
                    count_client --;
                    return 1;
                }
                else {
                   remove_spaces(buffer);
                    // Gửi kết quả về cho client
                    char response[2048];
                    sprintf(response, "Xin chào. Hiện có %d clients đang kết nối.Xâu bạn đã gửi là: %s", count_client, buffer);
                    send(sd, response, strlen(response), 0);
                }
            }
        }
    }

    return 0;
}