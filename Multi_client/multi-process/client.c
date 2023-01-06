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
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // create client socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    // connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server.\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        // send message to server
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

        // receive message from server
        recv(client_socket, &buffer, BUFFER_SIZE, 0);
        printf("Echo message from the server: %s\n", buffer);
    }

    return 0;
}
