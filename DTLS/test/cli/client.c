#include <stdio.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

int main() {
  // Create a DTLS BIO
  BIO *bio = BIO_new(BIO_f_dtls());
  if (bio == NULL) {
    fprintf(stderr, "Error creating DTLS BIO\n");
    return 1;
  }

  // Set the remote host and port to connect to
  BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, "127.0.0.1:12345");

  // Create an SSL context
  SSL_CTX *ctx = SSL_CTX_new(DTLS_client_method());
  if (ctx == NULL) {
    fprintf(stderr, "Error creating SSL context\n");
    return 1;
  }

  // Set the certificate and private key for the SSL context
  if (SSL_CTX_use_certificate_file(ctx, "client.crt", SSL_FILETYPE_PEM) <= 0) {
    fprintf(stderr, "Error setting certificate file\n");
    return 1;
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, "client.key", SSL_FILETYPE_PEM) <= 0) {
    fprintf(stderr, "Error setting private key file\n");
    return 1;
  }

  // Create an SSL object
  SSL *ssl = SSL_new(ctx);
  if (ssl == NULL) {
    fprintf(stderr, "Error creating SSL object\n");
    return 1;
  }

  // Set the DTLS BIO for the SSL object
  SSL_set_bio(ssl, bio, bio);

  // Set the MTU for the DTLS connection
  SSL_set_mtu(ssl, 1000);

  // Perform the DTLS handshake
  int ret = SSL_do_handshake(ssl);
  if (ret != 1) {
    fprintf(stderr, "Error performing DTLS handshake\n");
    return 1;
  }

  // Write data to the server
  int len = SSL_write(ssl, "Hello from the client!", 23);
  if (len < 0) {
    fprintf(stderr, "Error writing to server\n");
    return 1;
  }
  printf("Sent %d bytes to server\n", len);

  // Read data from the server
  char buf[1024];
  len = SSL_read(ssl, buf, sizeof(buf));
  if (len < 0) {
    fprintf(stderr, "Error reading from server\n");
    return 1;
  }
  printf("Received %d bytes from server: %s\n", len, buf);

  // Clean up
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  BIO_free(bio);

  return 0;
}