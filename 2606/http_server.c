#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define MAX_BUFFER_SIZE 2048
void *client_thread(void *);

void signal_handler(int signo)
{
    wait(NULL);
}
void send_directory_listing(int client_socket, const char *file_path)
{
    DIR *dir;
    struct dirent *entry;
    char response[MAX_BUFFER_SIZE];

    // Open current directory
    if (strlen(file_path) == 0)
    {
        dir = opendir(".");
    }
    else
    {
        dir = opendir(file_path);
    }
    if (dir == NULL)
    {
        perror("Unable to open directory");
        return;
    }
    char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(client_socket, response_header, strlen(response_header), 0);
    // Begin HTML response
    strcpy(response, "<html><body>\n");

    // Loop through directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            // Create HTML hyperlink with appropriate formatting
            if (entry->d_type == DT_DIR)
            {
                sprintf(response + strlen(response), "<p><b><a href=\"%s/\">%s/</a></b></p>\n", entry->d_name, entry->d_name);
            }
            else
            {
                sprintf(response + strlen(response), "<p><i><a href=\"%s\">%s</a></i></p>\n", entry->d_name, entry->d_name);
            }
        }
    }

    // End HTML response
    strcat(response, "</body></html>");

    // Send response over TCP
    send(client_socket, response, strlen(response), 0);

    // Close directory
    closedir(dir);
}
void send_text_file_content(int client_socket, const char *file_path)
{
    char buffer[MAX_BUFFER_SIZE];

    // Open the file
    int file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor == -1)
    {
        perror("Failed to open file");
        return;
    }
    char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(client_socket, response_header, strlen(response_header), 0);
    // Read file content and send over TCP
    ssize_t bytes_read;
    while ((bytes_read = read(file_descriptor, buffer, sizeof(buffer))) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }

    // Close the file
    close(file_descriptor);
}

// create a function to send image file content
void send_image_file_content(int client_socket, const char *file_path, const char *extension)
{
    char buffer[MAX_BUFFER_SIZE];
    // Open the requested file in binary mode
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return;
    }
    char *response_header = "";

    // For JPEG files
    if (strcmp(extension, ".jpg") == 0)
    {
        response_header = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nConnection: close\r\n\r\n";
    }
    // For PNG files
    else if (strcmp(extension, ".png") == 0)
    {
        response_header = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nConnection: close\r\n\r\n";
    }

    send(client_socket, response_header, strlen(response_header), 0);
    printf("%s\n", response_header);

    // Read and send the file content
    while (1)
    {
        int len = fread(buffer, 1, sizeof(buffer), file);
        if (len <= 0)
        {
            break;
        }
        send(client_socket, buffer, len, 0);
    }

    // Close the file
    fclose(file);
}
void send_audio_file_content(int client_socket, const char *file_path)
{
    char buffer[MAX_BUFFER_SIZE];
    FILE *f = fopen(file_path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char response_header[MAX_BUFFER_SIZE];
    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: audio/mp3\r\nConnection: keep-alive\r\n\r\n", fsize);
    send(client_socket, response_header, strlen(response_header), 0);

    while (1)
    {
        int len = fread(buffer, 1, sizeof(buffer), f);
        if (len <= 0)
            break;
        send(client_socket, buffer, len, 0);
    }
    fclose(f);
}
void handle_client_request(int client_socket, const char *request)
{
    // Extract the file or folder name from the request

    char file_path[MAX_BUFFER_SIZE];
    char method[16];
    sscanf(request, "%s %s", method, file_path);

    // Remove the first character '/' if it exists
    if (file_path[0] == '/')
    {
        memmove(file_path, file_path + 1, strlen(file_path));
    }
    // Remove the last character '/' if it exists
    if (strlen(file_path) > 0 && file_path[strlen(file_path) - 1] == '/')
    {
        file_path[strlen(file_path) - 1] = '\0';
    }

    printf("%s\n", file_path);
    // If the file path is empty, send the directory listing
    if (strcmp(file_path, "") == 0)
    {
        send_directory_listing(client_socket, file_path);
        return;
    }

    // Check if it is a folder or file
    DIR *dir = opendir(file_path);
    if (dir != NULL)
    {
        closedir(dir);
        // It is a folder, send the directory listing
        send_directory_listing(client_socket, file_path);
    }
    else
    {
        // It is a file, send the file content based on its extension
        const char *extension = strrchr(file_path, '.');
        printf("%s\n", extension);
        if (extension != NULL)
        {
            if (strcmp(extension, ".txt") == 0 || strcmp(extension, ".c") == 0 || strcmp(extension, ".cpp") == 0)
            {
                // Text file
                send_text_file_content(client_socket, file_path);
            }
            else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".png") == 0)
            {
                printf("image\n");
                send_image_file_content(client_socket, file_path, extension);
            }
            else if (strcmp(extension, ".mp3") == 0)
            {
                send_audio_file_content(client_socket, file_path);
            }
        }
    }
}
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

    signal(SIGPIPE, signal_handler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
            ;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf) - 1, 0);
    if (ret <= 0)
        return NULL;

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);
    handle_client_request(client, buf);

    close(client);
}