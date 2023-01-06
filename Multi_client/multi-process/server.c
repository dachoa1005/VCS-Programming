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
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // create server socket 
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    // bind server address to server socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // listen for incoming connection
    if (listen(server_socket, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connection...\n");

    while (1)
    {
        // accept incoming connection
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // create child process to handle new connection
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0)
        {
            // child process
            printf("New connection accepted from %s\n", inet_ntoa(client_addr.sin_addr));
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