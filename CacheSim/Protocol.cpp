#include <iostream>
#include <pthread.h>
#include <vector>
#include <assert.h>
#include "Protocol.h"
#include "MSI.h"
#include "Cache.h"


/**
 * Initialize the Protocol
 */
void Protocol::initialize(std::string protocol,
                                    std::vector<Cache> caches, int num_cores) {
    Protocol::num_cores = num_cores;
    Protocol::caches = caches;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&worker_cv, NULL);
    pthread_cond_init(&trace_cv, NULL);

    if(protocol == "MSI") {
        obj = MSI();
    } else {
        assert(0);
    }
}

/**
 *
 */
void Protocol::process_mem_access(int thread_id, std::string op,
                                            unsigned long addr) {
    pthread_mutex_lock(&lock);
    while(ready) {
        pthread_cond_wait(&trace_cv, &lock);
    }
    request_id = thread_id;
    request_op = op;
    request_addr = addr;
    ready = true;
    pthread_cond_broadcast(&worker_cv);
}
