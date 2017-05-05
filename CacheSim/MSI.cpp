#include <assert.h>
#include <pthread.h>
#include "MSI.h"
#include "Cache.h"
#include <iostream>

MSI::MSI() {
    for(int i = 0; i < num_cores; i++) {
        pthread_t tid;
        pthread_create(&tid, NULL, request_worker, (void *) ((long) i));
    }
}

void *MSI::request_worker(void *arg) {
    long tid = (long) arg;
    std::string op;
    unsigned long addr;

    while(true) {
        pthread_mutex_lock(&lock);
        //std::cout<<"Thread waiting\n";
        //fflush(stdout);
        while(!ready) {
            pthread_cond_wait(&worker_cv, &lock);
        }
        //std::cout<<"Thread done waiting\n";
        //fflush(stdout);
        if(request_id == tid) {
            op = Protocol::request_op;
            addr = Protocol::request_addr;

            ready = false;
            pthread_cond_signal(&Protocol::trace_cv);
            pthread_mutex_unlock(&lock);

            /* Lock is released before actually handling the request. This is
             * so that mem access processor can continue processing requests
             */
            handle_request(tid, op, addr);
        } else {
            pthread_mutex_unlock(&lock);
        }
        //std::cout<<"done handling\n";
        //fflush(stdout);
    }
    return NULL;
}

void MSI::handle_request(int tid, std::string op, unsigned long addr) {
    char status = Protocol::caches[tid].cache_status(addr);
    if(status != 'I') {
        Protocol::caches[tid].update_cache_lru(addr);
    } else {
        Protocol::caches[tid].insert_cache(addr, status);
    }
}
