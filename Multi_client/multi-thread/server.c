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

void *connection_handle(void *client_socket)
{
    int socket = *(int *)client_socket;
    char buffer[BUFFER_SIZE];

    int read_len;
    do {
        read_len = recv(socket, buffer, BUFFER_SIZE, 0);
        // end of string marker
        buffer[read_len] = '\0';
        if (strcmp(buffer, "exit") == 0)
            break;
        int send_status = send(socket, buffer, BUFFER_SIZE, 0);
        if (send_status < 0)
        {
            perror("Send failed");
            exit(EXIT_FAILURE);
        }
    } while (read_len > 0);

    return 0;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int server_socket, client_socket;
    struct sockaddr_in server_address;
    int addrlen = sizeof(server_address);
    char buffer[BUFFER_SIZE] = {0};
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created.\n");

    // Set socket option
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind socket to port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket bind to port %d.\n", port);

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (thread_count < MAX_CLIENTS)
    {
        printf("Listening on port %d\n", port);
        // Accept connection from client
        client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
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