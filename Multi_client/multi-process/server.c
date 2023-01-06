#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1000

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int n;

    while (1)
    {
        // receive message from client
        memset(buffer, 0, BUFFER_SIZE);
        n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (n < 0)
        {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }
    }
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
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    char buffer[BUFFER_SIZE] = {0};
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
    int client_count = 0;
    while (client_count < MAX_CLIENTS)
    {
        // accept incoming connection
        printf("Listening for incoming connection...\n");

        int client_addr_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Connection number: %d accepted from %s\n", client_count + 1, inet_ntoa(client_address.sin_addr));
        // create child process to handle new connection
        client_count++;

        // create child process to handle new connection
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0)
        {
            // child process
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        } else {
            // parent process
            close(client_socket);
        }
    }
    
    return 0;
}

