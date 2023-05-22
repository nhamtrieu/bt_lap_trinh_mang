#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/select.h>


#define MAX_MSG_LEN 1024

int main(int argc, char *argv[]) {  
  if (argc < 4) {
    printf("Usage: %s <ip-address> <recv-port> <send-port>\n", argv[0]);
    exit(1);
  }

  int recv_sockfd, send_sockfd;
  struct sockaddr_in recv_addr, send_addr;
  char buffer[MAX_MSG_LEN];
  fd_set readfds;
  int max_fd;

  // Khởi tạo socket nhận
  recv_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (recv_sockfd < 0) {
    perror("Error creating recv socket");
    exit(1);
  }

  // Thiết lập địa chỉ IP và cổng của máy nhận
  memset(&recv_addr, 0, sizeof(recv_addr));
  recv_addr.sin_family = AF_INET;
  recv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  recv_addr.sin_port = htons(atoi(argv[2]));

  // Gắn địa chỉ IP và cổng vào socket nhận
  if (bind(recv_sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
   perror("Error binding recv socket");
    exit(1);
  }

  // Khởi tạo socket gửi
  send_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (send_sockfd < 0) {
    perror("Error creating send socket");
    exit(1);
  }

  // Thiết lập địa chỉ IP và cổng của máy chờ
  memset(&send_addr, 0, sizeof(send_addr));
  send_addr.sin_family = AF_INET;
  send_addr.sin_addr.s_addr = inet_addr(argv[1]);
  send_addr.sin_port = htons(atoi(argv[3]));

  // Nhập nội dung tin nhắn từ bàn phím và gửi đi
  printf("Enter message: ");
  fgets(buffer, MAX_MSG_LEN, stdin);
  sendto(send_sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));

  // Sử dụng hàm select để theo dõi các socket đang đợi để đọc hoặc ghi
  FD_ZERO(&readfds);
  FD_SET(recv_sockfd, &readfds);
  FD_SET(STDIN_FILENO, &readfds);
  max_fd = recv_sockfd > STDIN_FILENO ? recv_sockfd : STDIN_FILENO;

  while (1) {
    if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
      perror("Error in select");
      exit(1);
    }

    // Kiểm tra xem socket nhận có sẵn để đọc không
    if (FD_ISSET(recv_sockfd, &readfds)) {
      memset(buffer, 0, MAX_MSG_LEN);
      if (recvfrom(recv_sockfd, buffer, MAX_MSG_LEN, 0, NULL, NULL) < 0) {
        perror("Error receiving message");
        exit(1);
      }
      printf("Received message: %s", buffer);
    }

    // Kiểm tra xem bàn phím có sẵn để đọc không
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      memset(buffer, 0, MAX_MSG_LEN);
      fgets(buffer, MAX_MSG_LEN, stdin);
      sendto(send_sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
    }

    // Cập nhật lại các socket cần theo dõi
    FD_ZERO(&readfds);
    FD_SET(recv_sockfd, &readfds);
    FD_SET(STDIN_FILENO, &readfds);
  }

  close(recv_sockfd);
  close(send_sockfd);
  return 0;
}