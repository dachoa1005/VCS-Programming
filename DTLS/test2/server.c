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
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(DTLS_PORT);

    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    SSL_load_error_strings(); // readable error messages
    SSL_library_init();       // initialize the library

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

    // load certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "sercert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "serkey.pem", SSL_FILETYPE_PEM) <= 0)
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
 
    BIO *readBIO = BIO_new_dgram(server_socket, BIO_NOCLOSE);
    BIO *writeBIO = BIO_new_dgram(server_socket, BIO_NOCLOSE);

    SSL_set_bio(ssl, readBIO, writeBIO);
    SSL_set_accept_state(ssl); // set to accept incoming connections

    // wait for client to connect
    printf("Waiting for client to connect...\n");
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = recvfrom(server_socket, NULL, 0, MSG_PEEK, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0)
    {
        perror("Unable to accept connection\n");
        exit(EXIT_FAILURE);
    }
    printf("Client accepted\n");

    // begin SSL handling
    char buffer[BUFFER_SIZE];
    int ret = 0;
    // ret = BIO_do_handshake(bio);

    do {
        // printf("SSL handshake...\n");
        // Gọi hàm SSL_do_handshake() để thực hiện SSL Handshake
        BIO_dgram_set_peer(writeBIO, (struct sockaddr *)&client_addr);
        ret = SSL_do_handshake(ssl);

        // Nếu cần đọc thêm dữ liệu từ socket, đọc dữ liệu và ghi vào readBIO
        if (ret == SSL_ERROR_WANT_READ)
        {
            int receivedBytes = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
            if (receivedBytes > 0)
            {
                printf("Client has received %d bytes data\n", receivedBytes);
                BIO_write(readBIO, buffer, receivedBytes);
            } else {
                printf("recvfrom() failed\n");
            }
        } else if (ret == SSL_ERROR_WANT_WRITE)
        {
            // Nếu cần gửi dữ liệu đi, đọc dữ liệu từ writeBIO và gửi đi
            int bytes_to_write = BIO_read(writeBIO, buffer, BUFFER_SIZE);
            if (bytes_to_write > 0)
            {
                printf("Server has received %d bytes data\n", bytes_to_write);
                sendto(server_socket, buffer, bytes_to_write, 0, (struct sockaddr *)&client_addr, client_addr_len);
            }
        }            
        // if (ret != 1)
        // {
        //     int err = SSL_get_error(ssl, ret);
        //     char *err_string = ERR_error_string(ERR_get_error(), NULL);
        //     printf("SSL error: %d\n", err);
        //     printf("SSL error string: %s\n", err_string);

        // }
    } while (ret != 1);
    printf("SSL handshake complete\n");
    close(client_socket);
    free(ssl);
    SSL_CTX_free(ctx);

    return 0;
}