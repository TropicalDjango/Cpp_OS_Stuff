#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>

#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>
#include <libgen.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int port;


void server() {
    int server_fd, socket, valread;
    struct sockaddr_in address;

    int op = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( (setsockopt(server_fd, SOL_SOCKET, 
                    SO_REUSEADDR | SO_REUSEPORT, 
                    &op, sizeof(op))) ) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
}

int main(int argc, char** argv) {
    if(argc == 2) {
        port = atoi(argv[1]);
        if(port >= 5000 || port <= 64000) {
            server();
        } else {
            perror("Incorrect port value, must be between 5000 and 64000");
        }
    } else {

    }
}
