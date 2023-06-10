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
    int client = *(int *)arg;
    int data = 0;
    strcpy(buf,"Enter username and password\n");
    send(client,buf,strlen(buf),0);
    while(1){
        data = recv(client,buf,sizeof(buf),0);
        int login = 0;
        if(data==0){
            close(client);
            return NULL;
        }
        buf[data-1] = 0;
        char username[10];
        char password[10];
        char somthing_else[10];
        int number_valid = sscanf(buf,"%s %s %s",username,password,somthing_else);
        if(number_valid!=2){
            strcpy(buf,"Please enter correct format username password\n");
            send(client,buf,strlen(buf),0);
            continue;
        }
        else{
            FILE * file = fopen("user_login.txt","r");
            while(!feof(file)){
                char name[10];
                char pass[10];
                fscanf(file,"%s %s",name,pass);
                if(strcmp(name,username)==0&&strcmp(pass,password)==0){
                    strcpy(buf,"Welcome client\n\0");
                    send(client,buf,strlen(buf),0);
                    login = 1;
                    break;
                }
            }
        if(login==1){
            break;
        }
        else{
            strcpy(buf,"Username or password not correct\n\0");
            send(client,buf,strlen(buf),0);
        }
        }
    }
    while(1){
        data = recv(client,buf,sizeof(buf),0);
        if(data<=0){
            printf("Connection lost\n");
            return NULL;
        }
        else{
            buf[data-1] = 0;
            char output[20] = " > out.txt";
            strcat(buf,output);
            system(buf);
            FILE * f = fopen("out.txt","r");
            while(fgets(buf,sizeof(buf),f)){
                send(client,buf,strlen(buf),0);
            }
            fclose(f);
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
        printf("Waiting for client to connect\n");
        int client = accept(sockfd,NULL,0);
        pthread_t t;
        pthread_create(&t,NULL,handle_client,&client);
        pthread_detach(t);
    }
}