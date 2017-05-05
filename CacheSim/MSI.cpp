#include <assert.h>
#include <pthread.h>
#include "MSI.h"
#include "Cache.h"

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

    pthread_mutex_lock(&lock);
    while(!ready) {
        pthread_cond_wait(&worker_cv, &lock);
    }
    if(request_id == tid) {
        op = Protocol::request_op;
        addr = Protocol::request_addr;

        ready = false;
        pthread_cond_signal(&Protocol::trace_cv);
        handle_request(tid, op, addr);
    }
    return NULL;
}

void MSI::handle_request(int tid, std::string op, unsigned long addr) {
    char status = Protocol::caches[tid].cache_status(addr);
    if(status == 'I') {
        Protocol::caches[tid].update_cache_lru(addr);
    } else {
        Protocol::caches[tid].insert_cache(addr, status);
    }
}
