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
#include <openssl/bio.h>
#include <openssl/ssl.h>

#define SERVER_PORT 12345

int main()
{
    // create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Error creating socket\n");
        return 1;
    }

    // configure the socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind the socket to a port
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        fprintf(stderr, "Error binding socket\n");
        return 1;
    }


    // Create a DTLS BIO
    BIO *bio = BIO_new_dgram(sockfd, 1); 
    if (bio == NULL)
    {
        fprintf(stderr, "Error creating DTLS BIO\n");
        return 1;
    }

    // Set the listen address and port
    BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, "127.0.0.1:12345");

    // Create an SSL context
    SSL_CTX *ctx = SSL_CTX_new(DTLS_server_method());
    if (ctx == NULL)
    {
        fprintf(stderr, "Error creating SSL context\n");
        return 1;
    }

    // Set the certificate and private key for the SSL context
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0)
    {
        fprintf(stderr, "Error setting certificate file\n");
        return 1;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        fprintf(stderr, "Error setting private key file\n");
        return 1;
    }

    // Create an SSL object
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        fprintf(stderr, "Error creating SSL object\n");
        return 1;
    }

    // Set the DTLS BIO for the SSL object
    SSL_set_bio(ssl, bio, bio);

    // Set the MTU for the DTLS connection
    // SSL_set_mtu(ssl, 1000);

    // Perform the DTLS handshake
    int ret = SSL_do_handshake(ssl);
    if (ret != 1)
    {
        fprintf(stderr, "Error performing DTLS handshake\n");
        return 1;
    }

    // Read data from the client
    char buf[1024];
    int len = SSL_read(ssl, buf, sizeof(buf));
    if (len < 0)
    {
        fprintf(stderr, "Error reading from client\n");
        return 1;
    }
    printf("Received %d bytes from client: %s\n", len, buf);

    // Write data to the client
    len = SSL_write(ssl, "Hello from the server!", 23);
    if (len < 0)
    {
        fprintf(stderr, "Error writing to client\n");
        return 1;
    }
    printf("Sent %d bytes to client\n", len);

    // Clean up
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    BIO_free(bio);

    return 0;
}
