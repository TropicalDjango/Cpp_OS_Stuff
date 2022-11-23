#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <bits/stdc++.h>

#include "tands.cpp"

#define MAX_BUFFER_SIZE 2
#define MAX_HOSTNAME 20
using namespace std;

int PORT;
char* ip_add;
char* hostname;
int pid = getpid();

void client_log() 
{
    
}

void client() {
    int sock = 0, valread, sock_fd;
    struct sockaddr_in saddr;
    char buffer[MAX_BUFFER_SIZE];
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket");
        exit(EXIT_FAILURE);
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, ip_add, &saddr.sin_addr) <= 0) {
        perror("IPv conversion");
        exit(EXIT_FAILURE);
    }

    if((sock_fd = connect(sock, (struct sockaddr*)&saddr,
                          sizeof(saddr))) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    while(true) 
    {
        cin.getline(buffer, MAX_BUFFER_SIZE);
        if(buffer[0] == 'T') {
            // send command to server
            send(sock, buffer, MAX_BUFFER_SIZE, 0);
            // read D<n> from server
            valread = read(sock, buffer, MAX_BUFFER_SIZE);
        } else if(buffer[0] == 'S') {
            Sleep(buffer[1]);
        }
        
    }
}

int main(int argc, char** argv) {
    if(argc == 3) {
        PORT = atoi(argv[1]);
        ip_add = argv[2];
        gethostname(hostname, MAX_HOSTNAME +1);

        // redirect output to logfile
        std::ofstream out(hostname + '.' + to_string(pid)); 
        // std::streambuf* coutbuf = std::cout.rdbuf();
        std::cout.rdbuf(out.rdbuf());

        if(PORT >= 5000 || PORT <= 64000) {
            client();
        } else {
            perror("Incorrect port value, must be between 5000 and 64000");
        }
    } else {
        perror("Need to specify port");
    }
}
