#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("Connected to server\n");

    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        printf("Enter a message:");
        fgets(buffer, 1024, stdin);
        buffer[strlen(buffer) - 1] = '\0';
        int send_status = send(client_socket, buffer, BUFFER_SIZE, 0);
        if (send_status < 0)
        {
            printf("Send failed\n");
            break;
        }

        if (strcmp(buffer, "exit") == 0)
            break;
        recv(client_socket, &buffer, BUFFER_SIZE, 0);
        printf("Echo message from the server: %s\n", buffer);
    }

    printf("Connection closed.\n");
    
    close(client_socket);
    return 0;
}
