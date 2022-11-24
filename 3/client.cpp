#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <bits/stdc++.h>

#include "tands.h"

#define MAX_BUFFER_SIZE 4
#define MAX_NAME 14
#define MAX_HOSTNAME 20
#define MAX_PID 6

using namespace std;

int PORT;
char* ip_add;
char name[MAX_NAME];
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
   
    // Make socket
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
    
    // connect to server
    if((sock_fd = connect(sock, (struct sockaddr*)&saddr,
                          sizeof(saddr))) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    cout << "Using port " << PORT << endl;
    cout << "Using server address " << ip_add << endl;
    cout << "Host " << hostname << endl;

    // send hostname.pid to server
    send(sock, hostname, MAX_HOSTNAME, 0);
    
    string  temp;
    while(cin >> temp) 
    {
        stringstream input(temp);
        char cmd;
        int cmd_n;
        input >> cmd;
        if(cmd == 'T') {
           
            // get number for command
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
        gethostname(name, MAX_NAME);

        if(PORT < 5000 || PORT > 64000) {
            perror("Incorrect port value");
        }

        // print is here to change where the output goes
        if(!print) {
            const char period[2] = ".";
            // create the full hostname.pid for the server
            strcat(hostname,name);
            strcat(hostname, period);
            strcat(hostname,to_string(pid).c_str());
            
            // redirect output to logfile
            ofstream out(hostname); 
            std::streambuf* coutbuf = std::cout.rdbuf();
            cout.rdbuf(out.rdbuf());
            
            // Logically you would think that this client call
            // is irrelevent but without this the output doesn't
            // get redirected if you know why please tell me
            client();
            cout.rdbuf(coutbuf);
            return 0;
        } else {
            client();
            return 0;
        }
        
    } else {
        perror("Need to specify port");
    }
}
