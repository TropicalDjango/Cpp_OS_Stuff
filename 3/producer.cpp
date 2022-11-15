#include "threader.h"
#include "producer.h"
#include "consumer.h"
#include "tands.h"
#include "extern.h"

/**
 *  producer() reads from std_in, parses command, 
 *  and when reaches EOF wait for all consumers to finish 
 *  and prints summary
 *
 *      @param  thread_handle   the array* for all thread ids
 *
 */

void producer(pthread_t *thread_handle) {
    
    int id = 0;
    std::string cmd;
    int n = 0;
    
    // keep reading from input till EOF is reached
    while(std::getline(std::cin, cmd) && !cmd.empty())
    {
        // reads S from input
        if(cmd[0] == 'S') { 
            cmd.erase(0,1);
            n = stoi(cmd);

            log_update(id, n, "Sleep");
            Sleep(n);
        // reads T
        } else if (cmd[0] == 'T') {
            cmd.erase(0,1);
            n = stoi(cmd);
             
            // Enter critical section
            pthread_mutex_lock(&info.work_mutex);
            
            info.shared_queue.push(n);
            
            // Signal waiting threads and unlock
            pthread_cond_signal(&info.work_cond);
            pthread_mutex_unlock(&info.work_mutex);
            
            log_update(id, n, "Work");
        }
    }
    
    // EOF reached, producer ends
    log_update(id, 0, "End");
    info.done = true;
    
    // signal anywaiting threads and wait till death
    pthread_cond_signal(&info.work_cond);
    for(int i = 0; i < info.threads; i++) {
        pthread_join(*(thread_handle+i),NULL);
    }

    // calling thread prints summary and exits
    summary();
    exit(0);
}


