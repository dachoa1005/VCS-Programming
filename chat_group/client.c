#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"

void *send_message(void *client_sockfd)
{
    int socket = *(int *)client_sockfd;
    char message[1024];

    // enter name and check then send to server
    while (1)
    {
        printf("Enter your name: ");
        fgets(message, 1024, stdin);
        message[strlen(message) - 1] = '\0';
        if (strcmp(message, "")!= 0)
            break;
    }
    if (send(socket, message, 1024, 0) < 0)
    {
        perror("send");
        exit(1);
    }

    while (1)
    {
        // printf("Enter message: ");
        fgets(message, 1024, stdin);
        message[strlen(message) - 1] = '\0';
        if (send(socket, message, 1024, 0) < 0)
        {
            perror("send");
            exit(1);
        }
    }
}

void *recv_message(void *client_sockfd)
{
    int socket = *(int *)client_sockfd;
    char message[1024];
    while (1)
    {
        int recv_len = recv(socket, message, 1024, 0);
        if (recv_len < 0)
        {
            perror("recv");
            exit(1);
        }
        if (recv_len == 0)
        {
            printf("Server disconnected\n");
            exit(1);
        }
        message[recv_len] = '\0';
        printf("%s\n", message);
    }
}

int main(int argc, char *argv[])
{
    pthread_t send_thread, recv_thread;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(port);

    if (connect(client_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Connected to server\n");

    pthread_create(&send_thread, NULL, send_message, (void *)&client_sockfd);
    pthread_create(&recv_thread, NULL, recv_message, (void *)&client_sockfd);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    return 0;
}