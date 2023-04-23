#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    if(connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Loi: ");
        return 1;
    }
    FILE *f ;
    f = fopen("text.txt", "r");
    if(f == NULL) {
        printf("Khong doc dc!\n");
        return 1;
    }
    char *content;
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    content = (char *) malloc(file_size);
    fread(content, 1, file_size, f);
    fclose(f);
    send(client, content, strlen(content), 0);
    close(client);
    return 0;
}