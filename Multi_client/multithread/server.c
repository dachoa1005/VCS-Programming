#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLIENTS 1000
#define BUFFER_SIZE 1024

void *connection_handle(void *arg);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created.\n");

    // Set socket option
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bind to port %d.\n", port);

    // Listen for incoming connections
    if (listen(server_fd, 1000) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (thread_count < MAX_CLIENTS)
    {
        printf("Listening on port %d\n", port);
        // Accept connection from client
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connection number: %d accepted.\n\n", thread_count+1);

        // create thread for each client
        if (pthread_create(&threads[thread_count], NULL, connection_handle,(void *)&client_socket) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        thread_count++;
    }

    // join threads 
    int i = 0;
    for (i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

void *connection_handle(void *client_socket)
{
    int socket = *(int *)client_socket;
    char server_msg[100] = "Hello from the server.\n";
    int send_status;
    send_status = send(socket, server_msg, BUFFER_SIZE, 0);
    if (send_status == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
    char client_msg[100];

    int read_len;
    do {
        read_len = recv(socket, client_msg, BUFFER_SIZE, 0);
        // end of string marker
        client_msg[read_len] = '\0';
        if (strcmp(client_msg, "exit") == 0)
            break;
        send_status = send(socket, client_msg, strlen(client_msg), 0);
    } while (read_len > 0);

    return 0;
}