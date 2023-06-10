#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>



void * handle_client(void * arg){
    char buf[256];
    int sockfd = *(int *)arg;
    int client = accept(sockfd,NULL,0);
    printf("New client accepted in process %d: %d\n", client, getpid());

    // Chờ dữ liệu từ client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
        return NULL;

    // Xử lý dữ liệu, trả lại kết quả cho client
    buf[ret] = 0;
    printf("Received from client %d: %s\n", client, buf);
    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);
    close(client);

}

int main(){
    int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    addr.sin_family = AF_INET;
    if(bind(sockfd,(struct sockaddr*) &addr,sizeof(addr))){
        printf("Error: %d - %s\n",errno,strerror(errno));
    }
    if(listen(sockfd,5)==-1){
        printf("Error: %d - %s\n",errno,strerror(errno));
    }
    char buf[256];
    int num_threads = 8;
    pthread_t thread_ids[num_threads];
    for (int i = 0; i < num_threads; i++) {
        int ret = pthread_create(&thread_ids[i], NULL, handle_client, &sockfd);
        if (ret != 0)
            printf("Could not create new thread.\n");
        sched_yield();
    }
    for (int i = 0; i< num_threads; i++)
        pthread_join(thread_ids[i], NULL);
    return 0;
}
    