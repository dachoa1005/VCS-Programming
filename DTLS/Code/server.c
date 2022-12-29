#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <openssl/rand.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DTLS_PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    // Create a UDP socket for the DTLS connection
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(DTLS_PORT);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket\n");
        return 1;
    } else {
        printf("Socket created\n");
    }

    // Bind the socket to a port
    int bind_status = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_status < 0){
        perror("Error binding socket");
        return 1;
    } else {
        printf("Socket binded\n");
    }

  // Initialize the DTLS context and set up the server or client SSL object
    SSL_library_init();
    int server = 1;
    // Create the DTLS context
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(DTLS_server_method());

    if (ctx == NULL)
    {
        printf("DTLS context not created\n");
        return -1;
    }

    // Create the server SSL object
    SSL *ssl = NULL;
    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        printf("Server SSL object not created");
        return -1;
    } else {
        printf("Server SSL object created\n");
    }

    // Set the SSL object to use the socket for the DTLS connection
    if (!SSL_set_fd(ssl, sockfd)) 
    {
        fprintf(stderr, "Error setting socket for SSL object\n");
        return 1;
    } else {
        printf("Socket set for SSL object\n");
    }

    while (1)
    {
    // Wait for incoming DTLS connections
        printf("Waiting for connection...\n");
        if(SSL_accept(ssl) == 1) 
        {
            printf("Connection established\n");
            break;
        }
        else
        {
            printf("Connection failed\n");
        }
        continue;
        while (1)
        {
            // receive msg from client
            char buffer[BUFFER_SIZE];
            int bytes_read = SSL_read(ssl, buffer, BUFFER_SIZE);
            if (bytes_read < 0)
            {
                printf("Error reading from socket\n");
                return -1;
            }
            else
            {
                printf("Message received: %s\n", buffer);
            }

            // send msg to client
            char *msg = "Hello from server";
            int bytes_sent = SSL_write(ssl, msg, strlen(msg));
            if (bytes_sent < 0)
            {
                printf("Error writing to socket\n");
                return -1;
            }
            else
            {
                printf("Message sent: %s\n", msg);
            }
        }
    }

    close(sockfd);
    return 0;
}