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
    pthread_mutex_lock(&lock);
    while(!ready) {
        pthread_cond_wait(&worker_cv, &lock);
    }
    if(request_id == tid) {
        ready = false;
        pthread_cond_signal(&trace_cv);
        handle_request(tid);
    }
    return NULL;
}

void MSI::handle_request(int tid) {
    char status = caches[tid].cache_status(request_addr);
    if(status == 'I') {
        caches[tid].update_cache_lru(request_addr);
    } else {
        caches[tid].insert_cache(request_addr, status);
    }
}
