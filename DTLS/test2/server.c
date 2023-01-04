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
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(DTLS_PORT);

    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }


    SSL_load_error_strings(); //readable error messages
    SSL_library_init(); //initialize the library

    const SSL_METHOD *method;
    SSL_CTX *ctx;

    // create a new DTLS context
    method = DTLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context\n");
        exit(EXIT_FAILURE);
    }

    // SSL_CTX_set_options(ctx, SSL_);

    // load certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // verify certificate
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key and certificate do not match\n");
        return 1;
    }

    // create new SSL
    SSL *ssl = SSL_new(ctx);
    if (!ssl)
    {
        perror("Unable to create SSL\n");
        exit(EXIT_FAILURE);
    }
    
    BIO *readBIO = BIO_new(BIO_s_mem());
    BIO *writeBIO = BIO_new(BIO_s_mem());
    SSL_set_bio(ssl, readBIO, writeBIO);
    SSL_set_accept_state(ssl); // set to accept incoming connections

    printf("Waiting for client to connect...\n");
    int client_addr_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_socket < 0)
    {
        perror("Unable to accept connection\n");
        exit(EXIT_FAILURE);
    }
    printf("Client accepted\n");

    // begin SSL handling
    char buf[BUFSIZE];
    while(!SSL_is_init_finished(ssl))
    {
        SSL_do_handshake(ssl);

        int bytes_write = BIO_read(writeBIO, buf, BUFSIZE);
        if (bytes_write > 0)
        {
            printf("Sending %d bytes to client\n", bytes_write);
            sendto(client_socket, buf, bytes_write, 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
        else
        {
            int bytes_received = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
            if (bytes_received > 0)
            {
                printf("Received %d bytes from client\n", bytes_received);
                BIO_write(readBIO, buf, bytes_received);
            }
        }
    }

    printf("SSL handshake complete\n");

    while (1)
    {
        // receive data from client
        int bytes_received = recvfrom(client_socket, buf, BUFSIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received > 0)
        {
            printf("Received %d bytes from client\n", bytes_received);
            BIO_write(readBIO, buf, bytes_received);
        }

        // read data from SSL
        int bytes_read = SSL_read(ssl, buf, BUFSIZE);
        if (bytes_read > 0)
        {
            printf("Received %d bytes from client\n", bytes_read);
            printf("Received: %s\n", buf);
        }

        // write data to SSL
        int bytes_write = BIO_read(writeBIO, buf, BUFSIZE);
        if (bytes_write > 0)
        {
            printf("Sending %d bytes to client\n", bytes_write);
            sendto(client_socket, buf, bytes_write, 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
    }
    close (client_socket);
    EVP_cleanup();
    return 0;
}