#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

struct sockaddr_in serv_addr;

void *create_connection(void *arg)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n");

    char buffer[BUFFER_SIZE];

    while (1)
    {
        //send message to server
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
            exit(1);
        
        // receive message from server
        recv(client_socket, &buffer, BUFFER_SIZE, 0);
        printf("Echo message from the server: %s\n", buffer);
    }

    printf("Connection closed.\n");
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    pthread_t threads[1000];

    for (int i = 0; i < 100; i++)
    {
        pthread_create(&threads[i], NULL, create_connection, NULL);
    }

    for (int i = 0; i < 100; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // close(client_socket);
    return 0;
}
