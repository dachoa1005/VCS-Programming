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

int main(int argc, char const *argv[])
{
    struct protoent *proto;
    proto = getprotobyname("UDP");
    if (proto == NULL)
    {
        printf("Error getting protocol\n");
        return 1;
    }
    printf("Protocol: %d\n", proto->p_proto);
}