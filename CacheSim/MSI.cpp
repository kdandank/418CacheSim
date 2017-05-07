#include <assert.h>
#include <pthread.h>
#include "MSI.h"
#include "Cache.h"
#include "Bus.h"
#include "Memory.h"
#include "Protocol.h"
#include <iostream>

MSI::MSI(int cache_id) {
    id = cache_id;
    pthread_t tid;
    pthread_create(&tid, NULL, request_worker, (void *) this);
    pthread_create(&tid, NULL, response_worker, (void *) this);
}

void *MSI::response_worker(void *arg) {
    MSI *obj = (MSI *) arg;
    pthread_mutex_lock(&Bus::resp_lock);
    while(true) {
        while(!Bus::pending_work[obj->id]) {
            pthread_cond_wait(&Bus::resp_cvar, &Bus::resp_lock);
        }
        pthread_mutex_unlock(&Bus::resp_lock);

        pthread_mutex_lock(&obj->lock);

        if(obj->pending_addr == Bus::addr) {
            assert(obj->pending_addr);
            Bus::recv_nak = true;
        } else {
            char status = obj->cache.cache_check_status(Bus::addr);
            switch(status) {
                case 'M':
                    if(Bus::opt == BusRd) {
                        obj->cache.cache_set_status(Bus::addr, 'S');
                    } else {
                        obj->cache.cache_set_status(Bus::addr, 'I');
                    }
                    break;
                case 'S':
                    if(Bus::opt == BusRdX) {
                        obj->cache.cache_set_status(Bus::addr, 'I');
                    }
                    break;
                case 'I':
                    break;
                default:
                    assert(0);
            }
        }
        pthread_mutex_unlock(&obj->lock);

        pthread_mutex_lock(&Bus::resp_lock);
        Bus::pending_work[obj->id] = false;
        Bus::resp_count++;
        if(Bus::resp_count == Protocol::num_cores - 1) {
            pthread_cond_signal(&Bus::req_cvar);
        }
    }
}

void *MSI::request_worker(void *arg) {
    MSI *obj = (MSI *)arg;
    std::string op;
    unsigned long addr;

    while(true) {
        pthread_mutex_lock(&Protocol::lock);
        //std::cout<<"Thread waiting\n";
        //fflush(stdout);
        while(!Protocol::ready) {
            pthread_cond_wait(&Protocol::worker_cv, &Protocol::lock);
        }
        //std::cout<<"Thread done waiting\n";
        //fflush(stdout);
        if(Protocol::request_id == obj->id) {
            op = Protocol::request_op;
            addr = Protocol::request_addr;

            Protocol::ready = false;
            pthread_cond_signal(&Protocol::trace_cv);
            pthread_mutex_unlock(&Protocol::lock);

            /* Lock is released before actually handling the request. This is
             * so that mem access processor can continue processing requests
             */
            handle_request(obj, op, addr);
        } else {
            pthread_mutex_unlock(&Protocol::lock);
        }
        //std::cout<<"done handling\n";
        //fflush(stdout);
    }
    return NULL;
}

void MSI::handle_request(MSI *obj, std::string op, unsigned long addr) {

    bool done = false;

    pthread_mutex_lock(&obj->lock);
    char status = obj->cache.cache_check_status(addr);
    if (status == 'M') {
        obj->cache.update_cache_lru(addr);
        pthread_mutex_unlock(&obj->lock);
        return;
    }
    pthread_mutex_unlock(&obj->lock);

    while(!done) {
        pthread_mutex_lock(&Bus::req_lock);
        pthread_mutex_lock(&obj->lock);
        status = obj->cache.cache_check_status(addr);

        switch(status) {
            case 'S':
                /* Just need to update cache lru on read */
                obj->cache.update_cache_lru(addr);
                if(op == "W") {
                    Bus::wait_for_responses(addr, BusRdX);
                    if(Bus::recv_nak != true) {
                        done = true;
                        obj->cache.cache_set_status(addr, 'M');
                    }
                }
                break;
            case 'I':
                if(op == "R") {
                    Bus::wait_for_responses(addr, BusRd);
                    if(Bus::recv_nak != true) {
                        done = true;
                        obj->cache.insert_cache(addr, 'S');
                        obj->pending_addr = addr;
                    }
                } else {
                    Bus::wait_for_responses(addr, BusRdX);
                    if(Bus::recv_nak != true) {
                        done = true;
                        obj->cache.insert_cache(addr, 'M');
                        obj->pending_addr = addr;
                    }
                }
                break;
            default:
                assert(0);
        }
        pthread_mutex_unlock(&obj->lock);
        pthread_mutex_unlock(&Bus::req_lock);

        if(status == 'I' && done) {
            Memory::request(addr);
            pthread_mutex_lock(&obj->lock);
            obj->pending_addr = 0;
            pthread_mutex_unlock(&obj->lock);
        }
    }
}
