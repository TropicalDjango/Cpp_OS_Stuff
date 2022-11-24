#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include <stdio.h>

#include <fcntl.h>

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

#include <sys/time.h>
#include <sys/types.h>

#include "tands.h"

#define BUFFER_SIZE 2
#define MAX_CLIENTS 10
#define MAX_HOSTNAME 20
using namespace std;

int port;
int trans_n = 1;
bool exit_cond = false, print = true;
int pid = getpid();
char client_name[MAX_CLIENTS][MAX_HOSTNAME];
int trans_client[MAX_CLIENTS];

time_t start_time = clock_t();


void server_end()
{
    auto lifetime = (clock_t() - start_time)/(double)CLOCKS_PER_SEC;
    cout << "SUMMARY" << endl;
    for(int ii; ii < MAX_CLIENTS; ii++)
    {
        cout << "   " << trans_client[ii] << " transactions";
        cout << " from " << client_name[ii] << endl;
    }

    int avg_trans = trans_n/lifetime;
    cout << fixed << setprecision(2);
    
    cout << avg_trans << " transactions/sec ("; 
    cout << trans_n << "/" << lifetime << ")";
    _exit(0);
}



void server_log(char cmd, int n, int client_index) 
{
    time_t epox = time(nullptr);
    cout << epox << ": #  " << trans_n << " (";
    if(cmd == 'D') {
        cout << "DONE) ";
    } else if(cmd == 'T') {
        cout << "T " << trans_n << ") ";
    }
    cout << "from " << client_name[client_index] << endl;
}

void sservice(int server_fd, int* client_socket, int addrlen,
              struct timeval timeout, struct sockaddr_in address) 
{
    int valread;
    char buffer[BUFFER_SIZE];
   
    while(1) 
    {
        fd_set rfds;
        // clear fd set
        FD_ZERO(&rfds);
        // add server socket to set
        FD_SET(server_fd, &rfds);
        fcntl(server_fd, O_NONBLOCK);
        // add clients sockets to set
        for(int ii; ii < MAX_CLIENTS; ii++) 
        {
            if(client_socket[ii] > 0)
                FD_SET(client_socket[ii], &rfds);
            if(client_socket[ii] > server_fd)
                server_fd = client_socket[ii];
        }
        // wait for changes in any of the sockets
        int file_stat = select(server_fd + 1, 
                                &rfds, NULL, NULL, &timeout);
        switch(file_stat) 
        {
            // Timeout
            case(0):
            {
                // close all sockets and exit
                close(server_fd);
                for(int ii = 0; ii < MAX_CLIENTS; ii++)
                {
                    close(client_socket[ii]);
                }
                server_end();
            }
            // Error
            case(-1):
            {
                perror("STATUS");
                break;
            }
            // activity on a socket
            default:
            {
                // server socket activity (i.e connection request)
                if(FD_ISSET(server_fd, &rfds)) 
                {
                    int new_socket;
                    if((new_socket = accept(server_fd,
                                    (struct sockaddr *)&address, 
                                    (socklen_t *)&addrlen)) < 0) 
                    {
                        perror("accept fail");
                        exit(EXIT_FAILURE);
                    }
                    // add new socket to client_sockets
                    for(int ii = 0; ii < MAX_CLIENTS; ii++) 
                    {
                        if(client_socket[ii]==0) 
                        {
                            client_socket[ii] = new_socket;
                            // get clientname and add to client_name[][]
                            read(new_socket, client_name[ii], 
                                    MAX_HOSTNAME);
                        }
                        break;
                    }
                }
                // client socket activity
                for(int ii = 0; ii < MAX_CLIENTS; ii++)
                {
                    if(FD_ISSET(client_socket[ii], &rfds))
                    {
                        // client disconnected
                        if((valread = read(client_socket[ii], 
                                          buffer, BUFFER_SIZE)) == 0)
                        {
                            // get client name
                            getpeername(client_socket[ii], 
                                        (struct sockaddr*)&address,
                                        (socklen_t*)&addrlen);
                            // close client socket
                            close(client_socket[ii]);
                            client_socket[ii] = 0;
                        }
                        // client message
                        else 
                        {
                            char cmd = buffer[0];
                            int cmd_n = buffer[1] - '0';
                            server_log(cmd, cmd_n, ii);
                            if(cmd == 'T') {
                                Trans(cmd_n);
                                trans_client[ii]++;
                                trans_n++;
                            }
                            std::string msg = "D" + 
                                            std::to_string(trans_n); 
                            server_log('D',cmd_n,ii);
                            send(client_socket[ii], 
                                 &msg, sizeof(msg), 0);
                        }
                    }
                }
            }
        }   
    }
}

void server() {
    struct sockaddr_in address;
    int server_fd, op = 1, 
        addrlen = sizeof(address),
        client_socket[MAX_CLIENTS];

    // initalize client sockets to 0
    for(int ii = 0; ii < MAX_CLIENTS; ii++) 
    {
        client_socket[ii] = 0; 
    }
    // this is to create a socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // This is too prevent error "address already in use"
    if( (setsockopt(server_fd, SOL_SOCKET, 
                    SO_REUSEADDR | SO_REUSEPORT, 
                    &op, sizeof(op))) ) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    // Setting the address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    cout << "Using port " << port << endl;
    
    // Setting up the timeout for 30 seconds
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    
    // Binding socket to address
    if(bind(server_fd, (struct sockaddr*)&address, 
                sizeof(address)) < 0 ) {
        perror("bind fail");
        exit(EXIT_FAILURE);
    }

    // Establish a maximum of MAX_CLIENTS connections
    if(listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen fail");
        exit(EXIT_FAILURE);
    }

    // After first client attempts connection
    sservice(server_fd, &client_socket[0], addrlen,
                   timeout, address);
}
                
                

int main(int argc, char** argv) {
    char hostname[MAX_HOSTNAME];
    if(argc == 2) {
        port = atoi(argv[1]);
        gethostname(hostname, MAX_HOSTNAME);
        if(!print) {
            // redirect output to logfile
            std::ofstream out(hostname + '.' + char(pid)); 
            // std::streambuf* coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
        }
        if(port >= 5000 || port <= 64000) {
            server();
        } else {
            perror("Incorrect port value, must be between 5000 and 64000");
        }
    } else {
        perror("Need to specify port");
    }
}
