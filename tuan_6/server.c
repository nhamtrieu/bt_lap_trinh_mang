#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<time.h>
#include<sys/ioctl.h>
#include<sys/select.h>
typedef struct sockaddr_in sockaddr_in;

typedef struct client_ifo{
    int client_accept;
    char client_id[100];
    char client_name[100];
}client_ifo;


int main(int argc,char * argv[]){
    sockaddr_in server;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi("9000"));
    //Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(sockfd==-1){
        printf("Error: %d - %s\n",errno, strerror(errno));
        printf("1\n");
        exit(1);
    }
    socklen_t server_len = sizeof(server);
    if(bind(sockfd,(struct sockaddr *)&server, server_len)){
        printf("Error: %d - %s\n",errno, strerror(errno));
        printf("2\n");
        exit(1);
    }
    if(listen(sockfd,5)==-1){
        printf("Failed to listen for incoming connections\n");
        exit(1);
    }

    int num_client = 0;
    client_ifo clients[100];
    char buf[256];
    fd_set fdread;
    while(1){
        FD_ZERO(&fdread);
        FD_SET(sockfd,&fdread);
        int maxdp = sockfd+1;
        int i = 0;
        for(; i < num_client; i++){
            FD_SET(clients[i].client_accept,&fdread);
            if(clients[i].client_accept+1> maxdp){
                maxdp = clients[i].client_accept+1;
            }
        }
        //Set time for waitting
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        printf("Waiting for somthing happen.\n");
        int ret = select(maxdp,&fdread,NULL,NULL,&timeout);
        if (ret < 0){
            printf("select() failed.\n");
            return 1;
        }
        if (ret == 0){
            printf("Timed out.\n"); continue;
        }
        //Event create, delete client
        if(FD_ISSET(sockfd,&fdread)){
            int client_id = accept(sockfd,NULL,NULL);
            while (1)
            {
                int data = recv(client_id,buf,sizeof(buf),0);
                if(data<=0){
                    printf("Something wrong!");
                }
                else{
                    client_ifo new_client;
                    buf[data] = 0;
                    if(strchr(buf,':')==NULL){
                        continue;
                    }
                    if(strchr(buf,' ')!=NULL){
                        continue;
                    }
                    char * ptr = strtok(buf,":");
                    strcpy(new_client.client_id,ptr);
                    ptr = strtok(NULL,":");
                    strcpy(new_client.client_name,ptr);
                    strcpy(buf,"Hello Client\n");
                    new_client.client_accept = client_id;
                    send(client_id,buf,strlen(buf),0);
                    clients[num_client++] = new_client;
                    break;
                }
            }
        }
        for(i = 0; i < num_client; i++){
            if(FD_ISSET(clients[i].client_accept,&fdread)){
                int data = recv(clients[i].client_accept,buf,sizeof(buf),0);
                if(data<=0){
                    printf("Something wrong!\n");
                    clients[i]=clients[num_client-1];
                    num_client--;
                }
                else{
                    buf[data] = 0;
                    char send_to_client[256];
                    memset(send_to_client, 0, sizeof(send_to_client));
                    strncpy(send_to_client,clients[i].client_id,strlen(clients[i].client_id));
                    strcat(send_to_client,":");
                    strcat(send_to_client,buf);
                    int j = 0;
                    for(; j < num_client; j++){
                        if(i!=j){
                            send(clients[j].client_accept,send_to_client,strlen(send_to_client),0);
                        }
                    }
                }
            }
        }
    }


    close(sockfd);
}