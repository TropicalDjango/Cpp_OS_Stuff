#include <string.h>
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
#include <poll.h>

#include <sys/time.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENT 8

int port;
int trans_n = 1;
bool exit_cond = false;
int client_fd[MAX_CLIENT];
int client_n = 0;

// void server_listen(void * ls);
// void server_service();

void server() {
    int valread;
    struct sockaddr_in address;

    int op = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // this is to create a socket
    if((client_fd[client_n] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // This is too prevent error "address already in use"
    if( (setsockopt(client_fd[client_n], SOL_SOCKET, 
                    SO_REUSEADDR | SO_REUSEPORT, 
                    &op, sizeof(op))) ) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    if(bind(client_fd[client_n], (struct sockaddr*)&address, 
                sizeof(address)) < 0 ) {
        perror("bind fail");
        exit(EXIT_FAILURE);
    }

    while(1) 
    {
        // wait(30s)
        // exit = true;
    }

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
        perror("Need to specify port");
    }
}
