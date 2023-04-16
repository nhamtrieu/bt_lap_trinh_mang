#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char mssv[10], hoten[100], ngaysinh[11], diemtb[5];

    if (argc != 3) {
        printf("Sử dụng: %s <địa chỉ IP của server> <cổng của server>\n", argv[0]);
        return -1;
    }

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("Không thể tạo socket\n");
        return -1;
    }

    // Thiết lập địa chỉ server
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("Địa chỉ IP không hợp lệ\n");
        return -1;
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Không thể kết nối tới server\n");
        return -1;
    }

    printf("Nhập thông tin sinh viên:\n");

    while(1) {
        // Nhập thông tin sinh viên
        printf("MSSV: ");
        scanf("%s", mssv);
        printf("Họ tên: ");
        scanf(" %[^\n]s", hoten);
        printf("Ngày sinh (yyyy-mm-dd): ");
        scanf("%s", ngaysinh);
        printf("Điểm trung bình: ");
        scanf("%s", diemtb);

        // Đóng gói thông tin sinh viên thành một chuỗi và gửi đến server
        char data[1024];
        sprintf(data, "%s %s %s %s", mssv, hoten, ngaysinh, diemtb);
        send(sock, data, strlen(data), 0);
        printf("Đã gửi thông tin sinh viên đến server\n");

        char options;
        printf("Tiếp tục?(Y/N): ");
        scanf(" %c", &options);
        if(options == 'N' || options == 'n'){
            printf("Kết thúc\n");
            break;
        }
    }

    // Đóng kết nối
    close(sock);
    return 0;
}