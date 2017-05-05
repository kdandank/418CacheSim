#include <iostream>
#include <pthread.h>
#include <vector>
#include <assert.h>
#include "Protocol.h"
#include "MSI.h"
#include "Cache.h"


pthread_mutex_t Protocol::lock;
pthread_cond_t Protocol::trace_cv;
pthread_cond_t Protocol::worker_cv;
Protocol Protocol::obj;
bool Protocol::ready;
int Protocol::request_id;
std::string Protocol::request_op;
unsigned long Protocol::request_addr;
int Protocol::num_cores;
std::vector<Cache> Protocol::caches;

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
    //std::cout<<"Protocol waiting\n";
    while(ready) {
        pthread_cond_wait(&trace_cv, &lock);
    }
    //std::cout<<"Protocol done waiting\n";
    request_id = thread_id;
    request_op = op;
    request_addr = addr;
    ready = true;
    pthread_cond_broadcast(&worker_cv);
    pthread_mutex_unlock(&lock);
}
