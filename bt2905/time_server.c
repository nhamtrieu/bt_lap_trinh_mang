#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>


void handle_close(int signo){
    int cid = wait(NULL);
    printf("%d closed\n",cid);
    return;
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
    int num_processes = 8;
    signal(SIGCHLD,handle_close);
    const char TIME_FORMAT[4][20] = {"dd/mm/yyyy","dd/mm/yy","mm/dd/yyyy","mm/dd/yy"};
    while(1){
        int client = accept(sockfd,NULL,0);
        if(fork()==0){
            close(sockfd);
            while(1){
                int data = recv(client,buf,sizeof(buf),0);
                buf[data-1] = 0; 
                int i;
                for(i = 0; i < 4; i++){
                    if(!strcmp(buf,TIME_FORMAT[i])){
                        break;
                    }
                }
                if(i==4){
                    strcpy(buf,"Please enter right format!\n\0");
                    send(client,buf,strlen(buf),0);
                }
                else{
                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        memset(buf,'\0',sizeof(buf));
                        switch (i)
                        {
                        case 0:
                            strftime(buf, sizeof(buf), "%d/%m/%Y", t);
                            break;
                        case 1:
                            strftime(buf, sizeof(buf), "%d/%m/%Y", t);
                            buf[6] = buf[8];
                            buf[7] = buf[9];
                            buf[8] = 0;
                            buf[9] = 0;
                            break;                        
                        case 2:
                            strftime(buf, sizeof(buf), "%m/%d/%Y", t);
                            break;                        
                        case 3:
                            strftime(buf, sizeof(buf), "%d/%m/%Y", t);
                            buf[6] = buf[8];
                            buf[7] = buf[9];
                            buf[8] = 0;
                            buf[9] = 0;
                            break;
                        default:
                            break;
                        }
                        buf[strlen(buf)] = '\n';
                        buf[strlen(buf)] = 0;
                        send(client,buf,strlen(buf),0);
                }
            }
            close(client);
            exit(EXIT_SUCCESS);
        
        }
        else{
            close(client);
        }
    }
}
