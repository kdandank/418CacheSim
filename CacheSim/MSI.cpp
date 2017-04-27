#include <assert.h>
#include <pthread.h>

MSI::MSI() {
    for(int i = 0; i < num_cores; i++) {
        int tid;
        pthread_create(&tid, NULL, request_worker, (void *) ((long) i));
    }
}

void *MSI::request_worker(void *arg) {
    long tid = (long) arg;
    pthread_mutex_lock(&lock);
    while(!ready) {
        pthread_cond_wait(&worker_cv);
    }
    if(request_id == tid) {
        ready = false;
        pthread_cond_signal(&trace_cv);
        handle_request();
    }
}

void MSI::handle_request(int tid) {
    char status = caches[tid].check_status(request_addr);
    if(status == 'I') {
        update_cache_lru(request_addr);
    } else {
        insert_cache(request_addr, status);
    }
}
