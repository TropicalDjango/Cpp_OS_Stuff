#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <bits/stdc++.h>

#include "tands.h"

#define MAX_BUFFER_SIZE 4
#define MAX_HOSTNAME 20
using namespace std;

int PORT;
char* ip_add;
char hostname[MAX_HOSTNAME];
int pid = getpid();
int transaction = 0;
bool print = false;

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
    
    cout << "Using port " << PORT << endl;
    cout << "Using server address " << ip_add << endl;
    cout << "Host " << hostname << endl;

    send(sock, hostname, MAX_HOSTNAME, 0);
    
    string  temp;
    while(cin >> temp) 
    {
        stringstream input(temp);
        char cmd;
        int cmd_n;
        input >> cmd;
        if(cmd == 'T') {
           
            // get all digits of filei
            input >> cmd_n;
            client_log(cmd, cmd_n);

            // send command to server
            send(sock, &cmd, sizeof(char), 0);
            send(sock, &cmd_n, sizeof(int), 0);
            transaction++;

            // read D<n> from server
            read(sock, &cmd_n, sizeof(int));
            client_log('D', cmd_n);

        } else if(cmd == 'S') {
            input >> cmd_n;
            cout << "Sleep " << cmd_n << " units" << endl;
            Sleep(cmd_n);
        }
    }
    cout << "Sent " << transaction << " Transactions" << endl;
    close(sock);
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
            return 0;
        } else {
            perror("Incorrect port value");
        }
    } else {
        perror("Need to specify port");
    }
}
