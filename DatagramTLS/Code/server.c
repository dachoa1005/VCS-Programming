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

#define DTLS_PORT 8880
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
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

    // Create a UDP socket for the DTLS connection
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket\n");
        return 1;
    } else {
        printf("Socket created\n");
    }

    // Bind the socket to a port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DTLS_PORT);
    
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        return 1;
    } else {
        printf("Socket binded\n");
    }

    // Set the socket as non-blocking
    // int flags = 1;
    // flags = ioctl(sockfd, FIONBIO, 0);
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // printf("....................");

    if (ssl == NULL)
    {
        printf("SSL object not created\n");
        return -1;
    } else {
        printf("SSL object created\n");
    }
    
    // Set the SSL object to use the socket for the DTLS connection
    if (!SSL_set_fd(ssl, sockfd)) 
    {
        fprintf(stderr, "Error setting socket for SSL object\n");
        return 1;
    } else {
        printf("Socket set for SSL object\n");
    }
    
    // printf("....................");

    // Set up the DTLS cookie generator
    // SSL_CTX_set_cookie_generate_cb(data.ctx, generate_cookie);
    // SSL_CTX_set_cookie_verify_cb(data.ctx, verify_cookie);

    // Wait for incoming DTLS connections
    while (1)
    {
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
    }

    close(sockfd);
    return 0;
}