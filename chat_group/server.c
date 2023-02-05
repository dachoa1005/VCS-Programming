#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5550
#define MAX_CLIENTS 100

typedef struct client_info
{
    int socket_fd;
    char name[100];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;

// listen for new connections
void listen_connections(int server_socket)
{
    while (1)
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        if (client_socket < 0)
        {
            perror("accept");
            exit(1);
        }

        // add client to clients array
        clients[num_clients].socket_fd = client_socket;
        sprintf(clients[num_clients].name, "Client %d", num_clients + 1);
        num_clients++;

        // create new process to handle this client
        pid_t pid = fork();
        if (pid == 0)
        {
            // child process
            char message[1000];
            while (1)
            {
                int n = recv(client_socket, message, sizeof(message), 0);
                if (n <= 0)
                {
                    break;
                } else {
                    message[n] = '\0';
                    printf("%s: %s", clients[num_clients - 1].name, message);
                }
                // forward message to all clients
                for (int i = 0; i < num_clients; i++)
                {
                    if (clients[i].socket_fd != client_socket)
                    {
                        send(clients[i].socket_fd, message, 1024, 0);
                    }
                }
            }

            close(client_socket);
            exit(0);
        }
        else if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    listen_connections(server_socket);

    return 0;
}