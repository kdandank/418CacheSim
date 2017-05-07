#include <assert.h>
#include <pthread.h>
#include "MSI.h"
#include "Cache.h"
#include "Bus.h"
#include "Protocol.h"
#include <iostream>

MSI::MSI(int cache_id) {
    id = cache_id;
    pthread_t tid;
    pthread_create(&tid, NULL, request_worker, (void *) this);
    pthread_create(&tid, NULL, response_worker, (void *) this);
}

void *MSI::response_worker(void *arg) {
}

void *MSI::request_worker(void *arg) {
    MSI *obj = (MSI *)arg;
    std::string op;
    unsigned long addr;

    while(true) {
        pthread_mutex_lock(&obj->lock);
        //std::cout<<"Thread waiting\n";
        //fflush(stdout);
        while(!Protocol::ready) {
            pthread_cond_wait(&Protocol::worker_cv, &obj->lock);
        }
        //std::cout<<"Thread done waiting\n";
        //fflush(stdout);
        if(Protocol::request_id == obj->id) {
            op = Protocol::request_op;
            addr = Protocol::request_addr;

            Protocol::ready = false;
            pthread_cond_signal(&Protocol::trace_cv);
            pthread_mutex_unlock(&obj->lock);

            /* Lock is released before actually handling the request. This is
             * so that mem access processor can continue processing requests
             */
            handle_request(obj, op, addr);
        } else {
            pthread_mutex_unlock(&obj->lock);
        }
        //std::cout<<"done handling\n";
        //fflush(stdout);
    }
    return NULL;
}

void MSI::handle_request(MSI *obj, std::string op, unsigned long addr) {
    pthread_mutex_lock(&Bus::req_lock);
    char status;// = obj->cache.cache_check_status(addr);

    switch(status) {
        case 'M':
            /* Just update lru count */
            obj->cache.update_cache_lru(addr);
            break;
        case 'S':
            if(op == "R") {
                obj->cache.update_cache_lru(addr);
            } else {

            }
            break;
        case 'I':
            if(op == "I") {
            } else {
            }
            break;
        default:
            assert(0);
    }
    pthread_mutex_unlock(&Bus::req_lock);


    if(status != 'I') {
    } else {
        obj->cache.insert_cache(addr, status);
    }
}
