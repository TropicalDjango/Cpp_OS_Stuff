#ifndef THREADER_H
#define THREADER_H

#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <fcntl.h>
#include <fstream>
#include <bits/stdc++.h>
#include <string.h>
#include <queue>
#include <pthread.h>
#include <vector>
#include <chrono>

using namespace std::chrono;

/**
 *  class Info is a container for all the global variables that 
 *  both producers and consumers need to access
 *
 *      @member log_file        the name of the log_file
 *      @member threads         number of threads
 *      @member id              the log_file id
 *      @member work            total producer work
 *      @member ask             total consumer asks
 *      @member receive         total consumer receives
 *      @member complete        total consumer completes
 *      @member sleep           total producer sleeps
 *      @member thread_info     tasks assigned to each consumer
 *      @member shared_queue    total work queue
 *      @member work_mutex      mutex for work queue
 *      @member log_mutex       mutex for log_file
 *      @member start           start time in seconds
 *      @member done            if the producer has reached EOF
 */

class Info
{
    public:
        std::string log_file;
        int threads, id;
        int work=0,ask=0,receive=0,complete=0,sleep=0;
        std::vector<int> thread_info;
        std::queue<int> shared_queue;

        pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_cond_t work_cond = PTHREAD_COND_INITIALIZER; 
        
        clock_t start = clock();

        bool done = false;
};

void log_update(int id, int n, std::string action);

void summary();

#endif
