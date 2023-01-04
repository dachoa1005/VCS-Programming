#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERV_PORT 1255
#define MAXLINE 255

int main() {
    int sockfd, n;
    socklen_t len;
    char mesg[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) >= 0) {
        printf("Server is running at port %d\n", SERV_PORT);
    } else {
        perror("Bind failed!");
        return 0;
    }


    while(1) {
        //create new socket to handle client
        len = sizeof(cliaddr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &len);
        if (newsockfd < 0) {
            perror("Accept failed!");
            return 0;
        }
        printf("Client connected: %s:%d\n", inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));

        //receive message from client
        n = recv(newsockfd, mesg, MAXLINE, 0);
        mesg[n] = 0;
        printf("Received: %s", mesg);

        //send message to client
        send(newsockfd, mesg, n, 0);
        printf("Sent: %s", mesg);
    }

    close(sockfd);
}