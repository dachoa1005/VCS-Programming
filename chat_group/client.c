#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT 5550

void send_message(int client_socket)
{
    char message[1024];
    while (1)
    {
        // printf("Enter message: ");
        fgets(message, 1024, stdin);
        if (send(client_socket, message, 1024, 0) < 0)
        {
            perror("send");
            exit(1);
        }
    }
}

void receive_message(int client_socket)
{
    char message[1024];
    while (1)
    {
        int recv_len = recv(client_socket, message, 1024, 0);
        if (recv_len < 0)
        {
            perror("recv");
            exit(1);
        }
        else if (recv_len == 0)
        {
            printf("Server disconnected\n");
            break;
        }
        else
        {
            message[recv_len] = '\0';
            printf("Client: %s", message);
        }
    }
}

int main(int argc, char *argv[])
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Connected to server\n");

    int pid = fork();
    if (pid == 0)
    {
        send_message(client_socket);
    }
    else
    {
        receive_message(client_socket);
    }

    return 0;
}