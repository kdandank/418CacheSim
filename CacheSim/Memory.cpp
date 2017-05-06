#include "Memory.h"

pthread_mutex_t Memory::lock;
std::list<MemRequest> Memory::req_table;
pthread_cond_t Memory::threads_cv;
pthread_cond_t Memory::req_cv;

MemRequest::MemRequest(unsigned long a) {
    addr = a;
    pthread_cond_init(&cv, NULL);
    done = false;
    waiters = 0;
}

void Memory::initialize() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&threads_cv, NULL);
    pthread_cond_init(&req_cv, NULL);

    pthread_t tid;
    pthread_create(&tid, NULL, memory_worker, NULL);
}

void *Memory::memory_worker(void *arg) {
    pthread_mutex_lock(&lock);
    while(true) {
        /* Wait while request table is empty */
        while(req_table.empty()) {
            pthread_cond_wait(&req_cv, &lock);
        }
        pthread_mutex_unlock(&lock);
        // wait logic - add latency
        //
        //
        pthread_mutex_lock(&lock);
        MemRequest &req = req_table.front();
        req.done = true;
        /* Multiple caches might be waiting on this */
        pthread_cond_broadcast(&req.cv);
        req_table.pop_front();
    }

    /* Never exits */
    pthread_mutex_unlock(&lock);
    return NULL;
}

void Memory::request(unsigned long addr) {
    pthread_mutex_lock(&lock);

    /* Check if request already exists in table */
    for(MemRequest& req: req_table) {
        if(req.addr == addr) {
            req.waiters++;
            while(!req.done) {
                pthread_cond_wait(&req.cv, &lock);
            }
            req.waiters--;
            /* The original requester can be woken up if there are no more
             * workers
             */
            if(req.waiters == 0) {
                pthread_cond_signal(&req.cv);
            }
            pthread_mutex_unlock(&lock);
            return;
        }
    }

    MemRequest req(addr);

    /* Insert request in table */
    req_table.push_back(req);

    /* Signal memory worker if request table was empty */
    if(req_table.empty()) {
        pthread_cond_signal(&req_cv);
    }

    /* Original thread waits till request is done and while there are no more
     * additional waiters
     */
    while(!req.done || req.waiters != 0) {
        pthread_cond_wait(&req.cv, &lock);
    }
    pthread_mutex_unlock(&lock);
}
