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

#define SERVER_IP "127.0.0.1"
#define DTLS_PORT 8888
#define BUFSIZE 1024

int main(int argc, char const *argv[])
{
    SSL_load_error_strings(); //readable error messages
    SSL_library_init(); //initialize the library

    SSL_CTX *ctx;
    ctx = SSL_CTX_new(DTLS_client_method());

    // load certificate and private key
    SSL_CTX_use_certificate_chain_file(ctx, "cert.pem");
    SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(DTLS_PORT);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    BIO *bio = BIO_new_dgram(sockfd, BIO_NOCLOSE); // create a new BIO
    SSL *ssl = SSL_new(ctx); // create a new SSL connection state
    BIO_ctrl(SSL_get_rbio(ssl), BIO_CTRL_DGRAM_SET_CONNECTED, 0, &server_addr);

    SSL_set_bio(ssl, bio, bio); // attach the socket to the SSL object

    // perform handshake
    int ret = SSL_connect(ssl);
    if (ret != 1)
    {
        printf("SSL_connect failed\n");
        return 0;
    }

    // send message
    char *msg = "Hello World!";
    int n = SSL_write(ssl, msg, BUFSIZE);
    if (n < 0)
    {
        printf("SSL_write failed\n");
        return 0;
    }

    // receive message
    char buf[BUFSIZE];
    n = SSL_read(ssl, buf, BUFSIZE);
    if (n < 0)
    {
        printf("SSL_read failed\n");
        return 0;
    }
    printf("Received: %s\n", buf);
    return 0;
}
