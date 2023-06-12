#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<pthread.h>
#include<dirent.h>
#include<unistd.h>

int main() {


    struct dirent *entry;
    DIR *dir = opendir("/lap_trinh_mang/bt_lap_trinh_mang/1206");
    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }
    char listItem[10][100];
    for(int i = 0; i < 10; i++){
        memset(listItem[i],0,sizeof(listItem[i]));
    }
    int noFile = 0;
    char end[3] = "\r\n";
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_type == DT_REG){
            strcpy(listItem[noFile++],entry->d_name);
        }
    }
    closedir(dir);


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
        int client = accept(sockfd,NULL,0);
        if(fork()==0){
            close(sockfd);
            memset(buf,0,sizeof(buf));
            char ok[3] = "OK";
            sprintf(buf,"%s %d %s",ok,noFile,end);
            send(client,buf,strlen(buf),0);
            for(int i = 0; i < noFile; i++){

                sprintf(buf,"%s %s",listItem[i],end);
                send(client,buf,strlen(buf),0);
            }
            strcpy(buf,"Vui long nhap ten file\n");
            send(client,buf,strlen(buf),0);
            while(1){
                int data = recv(client,buf,sizeof(buf),0);
                if(data<=0){
                    printf("Somthing happened\n");
                    break;
                }
                else{
                    buf[data-1]=0;
                    int check = 0;
                    int lenght = 0;
                    int index = 0;
                    for(int i = 0; i < noFile; i++){
                        if(strcmp(buf,listItem[i])==0){
                            check = 1;
                            FILE *fp;
                            index = i;

                            fp = fopen(listItem[index], "rb");
                            if (fp == NULL) {
                                perror("Error opening file");
                                return 1;
                            }

                            fseek(fp, 0L, SEEK_END);
                            lenght = ftell(fp);
                            fclose(fp);
                        }
                    }
                    if(check){
                        memset(buf,0,sizeof(buf));
                        sprintf(buf,"%s %d %s",ok,lenght,end);
                        send(client,buf,strlen(buf),0);
                            FILE *fp;

                            fp = fopen(listItem[index], "r");
                            while(fscanf(fp,"%s",buf)!=EOF) {
                                strcat(buf,"\n");
                                send(client,buf,strlen(buf),0);
                            }
                    }
                    else{
                        strcpy(buf,"Nhap ten file sai vui long nhap lai\n");
                        send(client,buf,strlen(buf),0);
                    }
                }
            }
        exit(EXIT_SUCCESS);
        }
        else{
            close(client);
        }
    }

}