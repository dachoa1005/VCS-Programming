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

int main(int argc, char const *argv[])
{
    // create socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Unable to create socket");
        return -1;
    }

    SSL_load_error_strings();
    SSL_library_init();

    //create context
    const SSL_METHOD *method = DTLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (ctx == NULL)
    {
        printf("Unable to create SSL context\n");
        return -1;
    }

    SSL_CTX_set_ecdh_auto(ctx, 1); // cấu hình tự động sử dụng giao thức ECDH - Elliptic Curve Diffie-Hellman
    // Giao thức ECDH là một giao thức khóa riêng có sử dụng đường cong eliptic để chia sẻ khóa giữa hai bên kết nối để mã hóa dữ liệu truyền giữa hai bên.

    // set key and cert
    if (SSL_CTX_use_certificate_file(ctx, "clicert.pem", SSL_FILETYPE_PEM) <= 0)
    {
        printf("Unable to set certificate file\n");
        return -1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "clikey.pem", SSL_FILETYPE_PEM) <= 0)
    {
        printf("Unable to set private key file\n");
        return -1;
    }

    // verify certificate
    if (!SSL_CTX_check_private_key(ctx))
    {
        printf("Private key and certificate do not match\n");
        return -1;
    }

    // create new SSL
    SSL *ssl = SSL_new(ctx);
    if (ssl == NULL)
    {
        printf("Unable to create SSL\n");
        return -1;
    }

    BIO* bio = BIO_new_dgram(client_socket, BIO_NOCLOSE);
    BIO *readBIO = BIO_new(BIO_s_mem());
    BIO *writeBIO = BIO_new(BIO_s_mem());

    SSL_set_bio(ssl, readBIO, writeBIO);
    SSL_set_connect_state(ssl); // set to connect to server


    // connect to server
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DTLS_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Unable to connect to server");
        return -1;
    }

    printf("Connected to server\n");

    // begin SSL handshake
    int ret = 0;
    char buffer[BUFSIZE];
    char buf[BUFSIZE];
    
    do {
        // Gọi hàm SSL_do_handshake() để thực hiện SSL Handshake
        ret = SSL_do_handshake(ssl);

        // Nếu cần đọc thêm dữ liệu từ socket, đọc dữ liệu và ghi vào readBIO
        if (ret == SSL_ERROR_WANT_READ) {
            int receivedBytes = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr*)&server_addr, &server_addr_len);
            if (receivedBytes > 0) {
                printf("Client has received %d bytes data\n", receivedBytes);
                BIO_write(readBIO, buffer, receivedBytes);
            }
        }

        // Nếu có dữ liệu mã hóa để gửi đi, gửi dữ liệu mã hóa đó đến server
        int bytesToWrite = BIO_read(writeBIO, buffer, BUFSIZE);
        if (bytesToWrite > 0) {
            printf("Client has %d bytes encrypted data to send\n", bytesToWrite);
            int bytes_sent = sendto(client_socket, buffer, bytesToWrite, 0, (struct sockaddr*)&server_addr, server_addr_len);
            if (bytes_sent >= 0)
            {
                printf("Sent %d bytes to server\n", bytes_sent);
            } else {
                perror("Unable to send data to server");
                exit(1);
            }
        }
    } while (ret != 1);

    printf("SSL handshake complete\n");

    close(client_socket);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    
    return 0;
}
