#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include<sys/select.h>

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    struct pollfd fds[FD_SETSIZE];
    memset(fds, 0, sizeof(fds));

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, FD_SETSIZE, 5000);

        if (ret< 0)
        {
            perror("poll() failed");
            return 1;
        }

        if (ret == 0)
        {
            printf("Timed out!!!\n");
            continue;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (i == 0)
                {
                    int client = accept(listener, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        fds[client].fd = client;
                        fds[client].events = POLLIN;
                        printf("New client connected: %d\n", client);
                    }
                    else
                    {
                        // Đang có quá nhiều kết nối
                        close(client);
                    }
                }
                else
                {
                    int ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", fds[i].fd, buf);
                    }
                }
            }
        }
    }

    close(listener);    

    return 0;
}