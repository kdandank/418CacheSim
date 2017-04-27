#include <iostream>
#include <pthread.h>
#include <vector>
#include <assert.h>
#include "Protocol.h"


/**
 * Initialize the Protocol
 */
static void Protocol::initialize(std::string protocol,
                                    std::vector<Cache> caches, int num_cores) {
    this.num_cores = num_cores;
    this.caches = caches;
    pthread_mutex_init(&lock);
    pthread_cond_init(&worker_cv);
    pthread_cond_init(&trace_cv);

    if(protocol == "MSI") {
        obj = MSI();
    } else {
        assert(0);
    }
}

/**
 *
 */
static void Protocol::process_mem_access(int thread_id, std::string op,
                                            unsigned long addr) {
    pthread_mutex_lock(&lock);
    while(ready) {
        pthread_cond_wait(&trace_cv);
    }
    request_id = thread_id;
    request_op = op;
    request_addr = addr;
    ready = true;
    pthread_cond_broadcast(&worker_cv);
}
