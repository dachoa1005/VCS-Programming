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

#define DTLS_PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    // Create a UDP socket for the DTLS connection
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
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

    // int bind_status = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    // if (bind_status < 0)
    // {
    //     printf("Socket not binded\n");
    //     return -1;
    // }
    // else
    // {
    //     printf("Socket binded\n");
    // }

    // Initialize the DTLS context and set up the server or client SSL object
    SSL_library_init();

    // Create the DTLS context
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(DTLS_client_method());
    if (ctx == NULL)
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
    ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        printf("Client SSL object not created\n");
        return -1;
    }
    else
    {
        printf("Client SSL object created\n");
    }
  

    // Set the socket to be used by the SSL object
    int setfd_status = SSL_set_fd(ssl, sockfd);
    if (setfd_status <= 0)
    {
        printf("Could not set socket to be used by SSL object\n");
        return -1;
    }
    else
    {
        printf("Socket set to be used by SSL object\n");
    }

    // Set the server address
    // SSL_set_connect_state(ssl);
    
    // Connect to the server
    int connect_status = SSL_connect(ssl);
    if (connect_status <= 0)
    {
        // printf("%d", connect_status);
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
