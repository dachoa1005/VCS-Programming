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
#include <openssl/err.h>

#define SERVER_IP "127.0.0.1"
#define DTLS_PORT 8888
#define BUFSIZE 1024

// int verify_cert(int ok, X509_STORE_CTX *ctx)
// {
//     char data[256];
//     X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
//     X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
//     printf("subject: %s \n", data);
//     X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
//     printf("issuer: %s \n", data);
//     return ok;
// }

// int generate_cookie(SSL *ssl, unsigned char *cookie, unsigned int *cookie_len)
// {
//     unsigned char *buffer, result[EVP_MAX_MD_SIZE];
//     unsigned int length = 0, resultlength;

//     // create buffer with address and port
//     BIO_dgram_get_peer(SSL_get_rbio(ssl), (struct sockaddr *)&peer);
//     length = sizeof(peer);
//     buffer = OPENSSL_malloc(length);
//     memcpy(buffer, &peer, length);

//     // generate hash
//     HMAC(EVP_sha1(), secret, secret_length, buffer, length, result, &resultlength);
//     OPENSSL_free(buffer);

//     // copy result to cookie
//     memcpy(cookie, result, resultlength);
//     *cookie_len = resultlength;
//     return 1;
// }

int main(int argc, char const *argv[])
{
    SSL_load_error_strings(); //readable error messages
    SSL_library_init(); //initialize the library

    SSL_CTX *ctx;

    // create a new DTLS context
    ctx = SSL_CTX_new(DTLS_server_method());

    // load certificate and private key
    SSL_CTX_use_certificate_chain_file(ctx, "cert.pem");
    SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);

    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_cert); 

    // SSL_CTX_set_cookie_generate_cb(ctx, generate_cookie);
    // SSL_CTX_set_cookie_verify_cb(ctx, verify_cookie);

    // connection setup
    /*
    Server need a socket awaiting for incoming connection
    Create a new BIO to respond to connection attempt
    */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(DTLS_PORT);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while(1)
    {
        BIO_ADDR *client_addr;
        BIO *bio = BIO_new_dgram(sockfd, BIO_NOCLOSE); // create a new BIO
        SSL *ssl = SSL_new(ctx); // create a new SSL connection state
        SSL_set_bio(ssl, bio, bio); // attach the socket to the SSL object

        // enable cookie exchange
        SSL_set_options(ssl, SSL_OP_COOKIE_EXCHANGE);

        // wait for a client to connect
        while (!DTLSv1_listen(ssl, client_addr))
        {
            // handle client connection
            int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
            bind(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
            connect(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));

            //set new fd and get BIO to connected
            BIO *cbio = SSL_get_rbio(ssl);
            BIO_set_fd(cbio, client_fd, BIO_NOCLOSE);
            BIO_ctrl(cbio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &server_addr);

            // finish handshake
            if (SSL_accept(ssl) <= 0)
            {
                ERR_print_errors_fp(stderr);
                break;
            } else {
                printf("Handshake completed\n");
            }

            // read data from client
            char buf[BUFSIZE];
            int len = SSL_read(ssl, buf, BUFSIZE);
            if (len > 0)
            {
                buf[len] = '\0';
                printf("Received: %s\n", buf);
            } else {
                ERR_print_errors_fp(stderr);
            }

            // write data to client
            char *msg = "Hello Client";
            SSL_write(ssl, msg, BUFSIZE);
        }   
    }
    return 0;
}
