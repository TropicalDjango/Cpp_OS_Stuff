#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <bits/stdc++.h>

#include "tands.h"

#define MAX_BUFFER_SIZE 2
#define MAX_HOSTNAME 20
using namespace std;

int PORT;
char* ip_add;
char hostname[MAX_HOSTNAME];
int pid = getpid();
int transaction = 0;
bool print = true;

void client_log(char cmd, int n) 
{
    time_t epox = time(nullptr);
    cout << epox << ": ";
    if(cmd == 'D') {
        cout << "Recv " << "(" << 
                "D  "<< n << ")" << endl;  
    } else {
         cout << "Send " << "(" << 
                "T  "<< n << ")" << endl;  
    }
}

void client() {
    int sock = 0, sock_fd;
    struct sockaddr_in saddr;
    char buffer[MAX_BUFFER_SIZE];
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client socket");
        exit(EXIT_FAILURE);
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);

    cout << "Using port " << PORT << endl;
    cout << "Using server address " << ip_add << endl;
    cout << "Host " << hostname << endl;

    if(inet_pton(AF_INET, ip_add, &saddr.sin_addr) <= 0) {
        perror("IPv conversion");
        exit(EXIT_FAILURE);
    }

    if((sock_fd = connect(sock, (struct sockaddr*)&saddr,
                          sizeof(saddr))) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    send(sock, hostname, MAX_HOSTNAME, 0);
    while(cin.getline(buffer, MAX_BUFFER_SIZE)) 
    {
        int cmd_n;
        if(buffer[0] == 'T') {
            cmd_n = buffer[1] - '0';
            client_log(buffer[0], cmd_n);
            // send command to server
            send(sock, buffer, MAX_BUFFER_SIZE, 0);
            transaction++;
            // read D<n> from server
            read(sock, buffer, MAX_BUFFER_SIZE);
            cmd_n = buffer[1]-'0';
            client_log(buffer[0], cmd_n);

        } else if(buffer[0] == 'S') {
            cmd_n = buffer[1]-'0';
            cout << "Sleep " << buffer[1] << "units" << endl;
            Sleep(cmd_n);
        }
    }
    char trans = transaction - '0';
    cout << "Sent " << trans << " Transactions";
    _exit(0);
}

int main(int argc, char** argv) {
    if(argc > 2) {
        PORT = atoi(argv[1]);
        ip_add = argv[2];
        gethostname(hostname, MAX_HOSTNAME);
        if(!print) {
            // redirect output to logfile
            std::ofstream out(hostname + '.' + to_string(pid)); 
            // std::streambuf* coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
        }
        if(PORT >= 5000 || PORT <= 64000) {
            client();
        } else {
            perror("Incorrect port value");
        }
    } else {
        perror("Need to specify port");
    }
}
