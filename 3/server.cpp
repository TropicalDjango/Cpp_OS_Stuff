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

#define BUFFER_SIZE 4
#define MAX_CLIENTS 10
#define MAX_HOSTNAME 20
#define MAX_NAME 14
#define MAX_PID 6

using namespace std;

int port;

// number of transactions
int trans_n = 0;

// true output goes to terminal, false goes to log file
const bool print = true;
const int pid = getpid();
const clock_t start_time = clock();

// names for each client
char client_name[MAX_CLIENTS][MAX_HOSTNAME];

// transactions for each client
int trans_client[MAX_CLIENTS];

/**
 *  server_end() is called when the server timesout and displays
 *  all relevent info for the server summary
 */
void server_end()
{
    clock_t stop_time = clock();
    double lifetime = double(stop_time - start_time)/CLOCKS_PER_SEC;

   fprintf(stdout, "\nSUMMARY:\n"); 
    
    // output client transaction and name if client name exists
    for(int ii = 0; 
        ii < MAX_CLIENTS && client_name[ii][0] != '\0'; 
        ii++)
    {
        fprintf(stdout, "   %3d transactions from %10s\n", trans_client[ii], client_name[ii]);
    }

    double avg_trans = trans_n/lifetime;
   
    fprintf(stdout, "%.2f transactions/second (%i/%g)\n", avg_trans, trans_n, lifetime);
}


/**
 *  server_log() outputs one of the transaction lines in the log
 *  depending on the provided arguments
 *
 *      @param  cmd             the action to print {'T','D'}
 *      @param  n               the number provided to Trans from client
 *      @param  client_index    the position of client in the 
 *                              client_socket[] array
 */
void server_log(char cmd, int n, int client_index) 
{
    auto epox = time(nullptr);

    // I assume large individual fprintf calls perform better than smaller seperate fprintfs
    if(cmd == 'D') {
      fprintf(stdout, "%d: #%3d (DONE) from %10s\n", epox, trans_n, client_name[client_index]);
    } else if(cmd == 'T') {
      fprintf(stdout, "%d: #%3d (T%3d) from %10s\n", epox, trans_n, n, client_name[client_index]);
    }
}

/**
 *  service is the main loop for the server. It is blocked by status()
 *  until an action on any socket is detected. 
 *
 *  Then it switches the result to find if timeout occured, 
 *  an error, or any socket activity. 
 *  
 *  There is server socket activity (connection request) or client socket.
 *  Client socket can either be disconnect or communication.
 *
 */

void sservice(int server_fd, int* client_socket, int addrlen,
              struct timeval timeout, struct sockaddr_in address) 
{
    while(1) 
    {
        fd_set rfds;
        
        // clear fd set
        FD_ZERO(&rfds);
        // reset timer
        timeout.tv_sec = 10;   
        // add server socket to set
        FD_SET(server_fd, &rfds);
        int max_sock = server_fd;
        
        // add clients sockets to set
        for(int ii = 0; ii < MAX_CLIENTS; ii++) 
        {
            if(client_socket[ii] > 0)
                FD_SET(client_socket[ii], &rfds);

            if(client_socket[ii] > max_sock)
                max_sock = client_socket[ii];
        }

        // wait for changes in any of the sockets with timeout 30s
        int file_stat = select(max_sock + 1, &rfds, NULL, NULL, &timeout);
        switch(file_stat)
        {
            // Timeout
            case(0):
            {
                // close all sockets and run server_end() to print summary
                close(server_fd);
                for(int ii = 0; ii < MAX_CLIENTS; ii++)
                {
                    close(client_socket[ii]);
                }
                server_end();
                return;
            }
            // Error
            case(-1):
            {
                perror("select error");
                break;
            }
            // activity on a socket
            default:
            {
                // server socket activity (i.e connection request)
                if(FD_ISSET(server_fd, &rfds)) 
                {
                    int new_socket = 0;
                    // connect to new client
                    if((new_socket = accept(server_fd,
                                    (struct sockaddr *)&address, 
                                    (socklen_t *)&addrlen)) < 0) 
                    {
                        perror("accept fail");
                        exit(EXIT_FAILURE);
                    }
                    // add new socket to client_socket[]
                    for(int ii = 0; ii < MAX_CLIENTS; ii++) 
                    {
                        // if empty spot is found
                        if(client_socket[ii]==0) 
                        {
                            client_socket[ii] = new_socket;
                            // get clientname and add to client_name[][]
                            read(new_socket, client_name[ii], MAX_HOSTNAME);
                            break;
                        }
                    }
                }
                // client socket activity
                for(int ii = 0; ii < MAX_CLIENTS; ii++)
                {
                    
                    char cmd;
                    int cmd_n = 0;
                    if(FD_ISSET(client_socket[ii], &rfds))
                    {
                        int cmd_read;
                        // client disconnected, try reading command into cmd
                        if((cmd_read = read(client_socket[ii], 
                                        &cmd, sizeof(char)) == 0))
                        {
                            // get client name
                            getpeername(client_socket[ii], 
                                        (struct sockaddr*)&address,
                                        (socklen_t*)&addrlen);
                            // close client socket
                            close(client_socket[ii]);
                            client_socket[ii] = 0;
                            
                            // fprintf(stdout, "Client %10s has disconnected\n", client_name[ii]);
                        }
                        // client message
                        else 
                        {

                            // read number from client, increment transaction
                            // count, send transaction number and output
                            read(client_socket[ii], &cmd_n, sizeof(int));
                            server_log(cmd, cmd_n, ii);

                            if(cmd == 'T') {
                                Trans(cmd_n);
                                trans_client[ii]++;
                                trans_n++;
                            }
                            server_log('D',trans_n,ii);
                            send(client_socket[ii], &trans_n, sizeof(trans_n), 0);
                        }
                    }
                }
            }
        }   
    }
}
/**
 *  Server initalizes the server socket and client sockets
 */
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

    fprintf(stdout, "Using port %i", port);
    
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
    sservice(server_fd, &client_socket[0], addrlen, timeout, address);
}
                
                

int main(int argc, char** argv) {
    char hostname[MAX_HOSTNAME];
    if(argc == 2) {
        port = atoi(argv[1]);
       
        if(!print) {
            char name[MAX_NAME];
            const char period = '.';

            strcat(hostname,name);
            strcat(hostname,&period);
            strcat(hostname,to_string(pid).c_str());
 
            // redirect output to logfile
            std::ofstream out(hostname + '.' + char(pid)); 
            std::streambuf* coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
            
            server();

            // restore stdout
            cout.rdbuf(coutbuf);
            return 0;
        }
        if(port >= 5000 || port <= 64000) {
            server();
            return 0;
        } else {
            perror("Incorrect port value, must be between 5000 and 64000");
        }
    } else {
        perror("Need to specify port");
    }
}
