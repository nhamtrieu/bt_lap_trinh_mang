#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<pthread.h>

typedef struct client_info{
    char id[20];
    char name[20];
    int addr;
    int index;
}client_info;

int num_clients = 0;
char message[100];
char current_time[20];
client_info list_clients[20];

pthread_mutex_t num_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void * handle_client(void *arg){
    client_info newclient;
    int client = *(int *)arg;
    char buf[256];
    strcpy(buf,"Please enter your id and name using format id:name\n");
    send(client,buf,strlen(buf),0);
    while (1)
        {
            int data = recv(client,buf,sizeof(buf),0);
            if(data<=0){
                printf("Something wrong!");
                return NULL;
            }
            else{
                buf[data-1] = 0;
                if(strchr(buf,':')==NULL){
                    continue;
                }
                if(strchr(buf,' ')!=NULL){
                    continue;
                }
                char * ptr = strtok(buf,":");
                strcpy(newclient.id,ptr);
                ptr = strtok(NULL,":");
                strcpy(newclient.name,ptr);
                strcpy(buf,"Hello Client\n");
                newclient.addr = client;
                send(client,buf,strlen(buf),0);
                pthread_mutex_lock(&num_clients_mutex);
                newclient.index = num_clients;
                list_clients[num_clients++] = newclient;
                pthread_mutex_unlock(&num_clients_mutex);
                break;
            }
    }
    while(1){
        int data = recv(client,buf,sizeof(buf),0);
        if(data<=0){
            printf("Client disconected!\n");
            pthread_mutex_lock(&num_clients_mutex);
            list_clients[newclient.index] = list_clients[num_clients-1];
            num_clients--;
            pthread_mutex_unlock(&num_clients_mutex);
            return NULL;
        }
        buf[data] = 0;
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time [100];
        strftime(time, sizeof(time), "%H:%M:%S ", t);
        strcat(time,newclient.name);
        strcat(time,": \0");
        strcat(time,buf);
        strcpy(buf,time);
        for(int i = 0; i < num_clients; i++){
            if(client!=list_clients[i].addr){
                send(list_clients[i].addr,buf,strlen(buf),0);
            }
        }
    }

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

    while(1){
        pthread_t t;
        int client = accept(sockfd,NULL,0);
        pthread_create(&t,NULL,handle_client,&client);
        pthread_detach(t);
    }
}   