#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>


int main(){
    //Khoi tao socket cho ket noi
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //Khai bao thong tin dia chi ben server
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    //Gan thong tin phia server vao socket
    if(bind(sockfd,(struct sockaddr *) &addr,sizeof(addr))){
        printf("Ma loi %d - %s",errno,strerror(errno));
    }
    //Chuyen socket vao che do lang nghe
    if(listen(sockfd,5)){
        printf("Ma loi %d - %s",errno,strerror(errno));
    }
    int num_processes = 8;
    char buf[256];

    // Tạo trước các tiến trình, mỗi tiến trình lặp lại công việc chấp nhận kết nối và xử lý yêu cầu của client
    for (int i = 0; i < num_processes; i++)
        if (fork() == 0)
            while(1)
            {
                // Chờ kết nối
                int client = accept(sockfd, NULL, NULL);
                printf("New client accepted in process %d: %d\n", client, getpid());

                // Chờ dữ liệu từ client
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    continue;

                // Xử lý dữ liệu, trả lại kết quả cho client
                buf[ret] = 0;
                printf("Received from client %d: %s\n", client, buf);
                char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
                send(client, msg, strlen(msg), 0);
                close(client);
            }

    // Đợi vô thời hạn, đảm bảo chương trình tiếp tục hoạt động
    wait(NULL);

    return 0;
}