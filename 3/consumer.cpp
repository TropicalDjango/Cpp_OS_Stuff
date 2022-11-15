#include "threader.h"
#include "consumer.h"
#include "producer.h"
#include "tands.h"
#include "extern.h"
/**
 *  consumer()'s job is to take n from work queue,
 *  run Trans(n), and exit when producer reached EOF
 *  and when queue has no more work
 *
 *      @params num     the id of running thread 
 *
 */

void *consumer(void *args) {
    int *num = (int *) args;

    while(1)
    {

        log_update(*num, 0, "Ask");

        // Enter critical section
        pthread_mutex_lock(&info.work_mutex);

        if(info.shared_queue.size() < 1) {
            // If producer has ended and queue is empty, unlock and exit
            if(info.done) {       
                
                pthread_mutex_unlock(&info.work_mutex);
                pthread_cond_signal(&info.work_cond);
                pthread_exit(0); 

            // If producer is running and queue is empty, wait for work
            } else {
                
                pthread_cond_wait(&info.work_cond,&info.work_mutex);
                // If producer has stopped and queue is still empty, unlock and exit
                if(info.shared_queue.size() < 1 && info.done) {
                    
                    pthread_mutex_unlock(&info.work_mutex);
                    pthread_cond_signal(&info.work_cond);
                    pthread_exit(0); 
                }
            }
        }
        
        int n = info.shared_queue.front();

        info.shared_queue.pop();

        // leave critical section
        pthread_mutex_unlock(&info.work_mutex);

        log_update(*num, n, "Receive");

        Trans(n);

        log_update(*num, n, "Complete");
    }
}
