#include "threader.h"
#include "consumer.h"
#include "producer.h"
#include "tands.h"
#include "extern.h"

#include <iomanip>
#include <chrono>
#include <fstream>
using namespace std::chrono;

Info info;

/**
 *  summary()'s job is to print all the information in the Info class
 *  and to calculate the transactions per seconds. 
 *
 *  pretty self explanitory.
 *
 */

void summary() {
    // get current time and subtract from start to approximatly get total runtime
    auto stop = clock();
    double runtime = ((double)(stop-info.start))/ CLOCKS_PER_SEC;

    std::cout << "Summary:" << std::endl;
    std::cout << "    " << "Work" << "             " << info.work << std::endl;
    std::cout << "    " << "Ask" << "              " << info.ask << std::endl;
    std::cout << "    " << "Receive" << "          " << info.receive << std::endl;
    std::cout << "    " << "Complete" << "         " << info.complete << std::endl;
    std::cout << "    " << "Sleep" << "            " << info.sleep << std::endl;
    for(int i = 0; i < info.threads; i++) 
    {
        std::cout << "    " << "Thread " << (i+1) << "         "; 
        std::cout << info.thread_info[i] << std::endl;
    }

    // set correct significant figures
    std::cout << std::fixed << std::setprecision(2);

    double microseconds = info.complete/runtime;
    std::cout << "Transactions per second: " << microseconds << std::endl;
   
}

/**
 *  log_update()'s job is to print out a single line containing all relevent 
 *  info about a thread event into std_out and to update all information 
 *  in class info
 *  
 *      @param id       int id of calling thread (producer_id = 0)
 *      @param n        int n value of command
 *      @param action   string action is the action that the thread took 
 *
 *  Assumptions: 
 *      1- std_out has been redirected to log file by threader
 *
 */

void log_update(int id, int n, std::string action) {
    pthread_mutex_lock(&info.log_mutex);
    
    auto stop = clock();
    double runtime = ((double)(stop-info.start))/ CLOCKS_PER_SEC;

    std::cout << std::fixed << std::setprecision(3) << "   ";
    std::cout << runtime << " ID= " << id << " ";

    if(!action.compare("Ask")) {
        std::cout << "     " << action << std::endl;
        info.ask += 1;
    
    } else if(!action.compare("Receive")) {
        std::cout << "Q= " << info.shared_queue.size() << " "; 
        std::cout << action << "     " << n << std::endl;
        info.receive += 1;

    } else if(!action.compare("Work")) {
        std::cout << "Q= " << info.shared_queue.size() << " "; 
        std::cout << action << "        " << n << std::endl;
        info.work += 1;
    
    } else if(!action.compare("Sleep")) {
        std::cout << "     " << action << "       " << n << std::endl;
        info.sleep += n;

    } else if(!action.compare("Complete")) {
        std::cout << "     " << action << "    " << n << std::endl;
        info.complete += 1;
        info.thread_info[id-1] += 1;

    } else if(!action.compare("End")) {
        std::cout << "     " << action << std::endl;
        info.done = true;
    } else {
        perror("log_update: Invalid Event");
    }
    pthread_mutex_unlock(&info.log_mutex);
}

/**
 *  threader()'s job is create all threads, redirect standard output
 *  to log file, and to become the producer.
 *
 */

void threader() {

    pthread_t *thread_handle = new pthread_t[info.threads];
    int *pids = new int[info.threads];
    info.thread_info.resize(info.threads);

    // create and redirect output to file
    info.log_file = "prodcon." + std::to_string(info.id) + ".log";
    std::ofstream out(info.log_file);
    std::cout.rdbuf(out.rdbuf());

    // create threads and store ids in thread_handle
    for(int i = 0; i < info.threads; i++) {
        pids[i] = i+1;
        pthread_create(thread_handle+i, NULL, consumer, pids+i);
    }

    producer(thread_handle);
}

/**
 *  main()'s job is to parse CL for valid number inputs, 
 *  update info, and run threader
 *
 */

int main(int argc, char** argv) {
	
	if(argc >= 3) {
		info.threads = atoi(argv[1]);
		info.id = atoi(argv[2]);
	} else if (argc == 2) {
		info.threads = atoi(argv[1]);
		info.id = 0;
	} else {
		std::cout << "need 3 arguments\n";
		exit(0);
	}

	threader();
}
