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
#include <openssl/err.h>
#include <netdb.h>

#define DTLS_PORT 8880
#define BUFFER_SIZE 1024

typedef struct
{
    SSL_CTX *ctx;
    char buf[BUFFER_SIZE];
} dtls_data;

int main(int argc, char const *argv[])
{
    dtls_data data;
    int server = 0;

    // Initialize the DTLS context and set up the server or client SSL object
    SSL_library_init();

    // Create the DTLS context
    data.ctx = SSL_CTX_new(DTLS_client_method());
    if (data.ctx == NULL)
    {
        printf("DTLS context not created\n");
        return -1;
    }
    else
    {
        printf("DTLS context created\n");
    }

    // Create the client SSL object
    SSL *ssl = NULL;
    ssl = SSL_new(data.ctx);
    if (ssl == NULL)
    {
        printf("Client SSL object not created\n");
        return -1;
    }
    else
    {
        printf("Client SSL object created\n");
    }
  
    // Set the socket to non-blocking
    // int flags = fcntl(sockfd, F_GETFL, 0);
    // if (flags < 0)
    // {
    //     printf("Could not get socket flags\n");
    //     return -1;
    // }
    // if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    // {
    //     printf("Could not set socket to non-blocking\n");
    //     return -1;
    // }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Thiết lập thông tin địa chỉ của server
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(DTLS_PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Create the socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Socket not created\n");
        return -1;
    }
    else
    {
        printf("Socket created\n");
    }

    // Set the socket to be used by the SSL object
    if (SSL_set_fd(ssl, sockfd) == 0)
    {
        printf("Could not set socket to be used by SSL object\n");
        return -1;
    }
    else
    {
        printf("Socket set to be used by SSL object\n");
    }

    // Connect to the server
    if (SSL_connect(ssl) == 0)
    {
        printf("Could not connect to server\n");
        return -1;
    }
    else
    {
        printf("Connected to server\n");
    }



    // Send a message to the server
    char *msg = "Hello from client";
    // int len = strlen(msg);
    int bytes_sent = SSL_write(ssl, msg, BUFFER_SIZE);
    if (bytes_sent < 0)
    {
        printf("Could not send message to server\n");
        return -1;
    }
    else
    {
        printf("Sent message to server\n");
    }

    // Receive a message from the server
    char buf[BUFFER_SIZE];
    int bytes_read = SSL_read(ssl, buf, BUFFER_SIZE);
    if (bytes_read < 0)
    {
        printf("Could not receive message from server\n");
        return -1;
    }
    else
    {
        printf("Received message from server: %s\n", buf);
    }

    // Close the socket
    close(sockfd);

    // Free the SSL object
    SSL_free(ssl);
    return 0;
}
