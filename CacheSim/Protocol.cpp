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
bool Protocol::ready;
int Protocol::request_id;
std::string Protocol::request_op;
unsigned long Protocol::request_addr;
int Protocol::num_cores;
std::vector<SnoopingCache> Protocol::sn_caches;

/**
 * Initialize the Protocol
 */
void Protocol::initialize(std::string protocol, int num_cores, int cache_size,
                                int associativity) {
    Protocol::num_cores = num_cores;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&worker_cv, NULL);
    pthread_cond_init(&trace_cv, NULL);

    /* Set cache properties */
    Cache::cache_init(cache_size, associativity);
    for(int i = 0; i < num_cores; i++) {
        if(protocol == "MSI") {
            sn_caches.push_back(MSI(i));
        } else {
            assert(0);
        }
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
    /* Reset the block bits of the address */
    request_addr = addr & ((1 << Cache::block_bits) - 1);
    ready = true;
    pthread_cond_broadcast(&worker_cv);
    pthread_mutex_unlock(&lock);
}
