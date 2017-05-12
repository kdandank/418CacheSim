/**
 * Implementation specific to the Competitive Snooping protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include <assert.h>
#include <pthread.h>
#include "CompSnooping.h"
#include "Cache.h"
#include "Bus.h"
#include "Memory.h"
#include "Protocol.h"
#include <iostream>

CompSnooping::CompSnooping(int cache_id) {
    id = cache_id;
    pthread_t tid;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&tid, NULL, request_worker, (void *) this);
    pthread_create(&tid, NULL, response_worker, (void *) this);
}

/**
 * Implementation for the response worker thread for each cache.
 */
void *CompSnooping::response_worker(void *arg) {
    CompSnooping *obj = (CompSnooping *) arg;
    pthread_mutex_lock(&Bus::resp_lock);
    while(true) {
        while(!Bus::pending_work[obj->id]) {
            pthread_cond_wait(&Bus::resp_cvar, &Bus::resp_lock);
        }
        pthread_mutex_unlock(&Bus::resp_lock);

        pthread_mutex_lock(&obj->lock);

        cache_state status = obj->cache.cache_check_status(Bus::addr);
        switch(status) {
            case Modified:
                assert(Bus::opt == BusRd);
                obj->cache.cache_set_status(Bus::addr, ShModified);
                Bus::owner_id = obj->id;
                Bus::read_ex = false;
                break;
            case ShModified:
                if(Bus::opt == BusRd) {
                    Bus::owner_id = obj->id;
                    Bus::read_ex = false;
                } else if(Bus::opt == BusUpdt) {
                    /* Invalidate if the processor does not access this address
                     * too often
                     */
                    if(obj->cache.cache_get_counter(Bus::addr) <= 1) {
                        obj->cache.cache_set_status(Bus::addr, Invalid);
                    } else {
                        obj->cache.cache_decr_counter(Bus::addr);
                        obj->cache.cache_set_status(Bus::addr, ShClean);
                        Bus::read_ex = false;
                    }
                }
                break;
            case Invalid:
                break;
            case ShClean:
                if(obj->pending_addr != Bus::addr) {
                    Bus::owner_id = obj->id;
                }
                /* Invalidate if the processor does not access this address
                 * too often
                 */
                if(Bus::opt == BusUpdt) {
                    if(obj->cache.cache_get_counter(Bus::addr) <= 1) {
                        obj->cache.cache_set_status(Bus::addr, Invalid);
                    } else {
                        obj->cache.cache_decr_counter(Bus::addr);
                        Bus::read_ex = false;
                    }
                } else {
                    Bus::read_ex = false;
                }
                break;
            case Exclusive:
                Bus::read_ex = false;
                assert(Bus::opt == BusRd);
                obj->cache.cache_set_status(Bus::addr, ShClean);

                if(obj->pending_addr != Bus::addr) {
                    Bus::owner_id = obj->id;
                }
                break;
            default:
                assert(0);
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

/**
 * Implementation for the request worker thread for each cache.
 */
void *CompSnooping::request_worker(void *arg) {
    CompSnooping *obj = (CompSnooping *)arg;
    std::string op;
    unsigned long addr;

    pthread_mutex_lock(&Protocol::lock);
    while(true) {
        while(!Protocol::ready || Protocol::request_id != obj->id) {
            pthread_cond_wait(&Protocol::worker_cv, &Protocol::lock);
        }
        std::cout<<"Thread "<<obj->id<< " got request\n";
        op = Protocol::request_op;
        addr = Protocol::request_addr;

        Protocol::ready = false;
        pthread_cond_signal(&Protocol::trace_cv);
        pthread_mutex_unlock(&Protocol::lock);

        /* Lock is released before actually handling the request. This is
         * so that mem access processor can continue processing requests
         */
        handle_request(obj, op, addr);
        std::cout<<"Thread " << obj->id<<" Done with request\n";
        pthread_mutex_lock(&Protocol::lock);
        fflush(stdout);
        Protocol::trace_count++;
    }
    pthread_mutex_unlock(&Protocol::lock);
    return NULL;
}

/**
 * This function defines how each and every cache state responds to different requests.
 */
void CompSnooping::handle_request(CompSnooping *obj, std::string op, unsigned long addr) {
    assert(addr);

    pthread_mutex_lock(&obj->lock);
    cache_state status = obj->cache.cache_check_status(addr);
    if ((status == Exclusive && op == "R") || status == Modified ||
            (status == ShClean && op == "R") ||
            (status == ShModified && op == "R")) {
        obj->cache.cache_incr_counter(addr);
        obj->cache.update_cache_lru(addr);
        pthread_mutex_unlock(&obj->lock);
        return;
    } else if(status == Exclusive && op == "W") {
        obj->cache.cache_incr_counter(addr);
        obj->cache.cache_set_status(addr, Modified);
        obj->cache.update_cache_lru(addr);
        pthread_mutex_unlock(&obj->lock);
        return;
    }
    pthread_mutex_unlock(&obj->lock);

    pthread_mutex_lock(&Bus::req_lock);
    pthread_mutex_lock(&obj->lock);
    status = obj->cache.cache_check_status(addr);

    Protocol::bus_transactions++;

    switch(status) {
        case ShClean:
            assert(op == "W");
            obj->cache.cache_incr_counter(addr);
            obj->cache.update_cache_lru(addr);
            obj->opt = BusUpdt;
            Bus::wait_for_responses(obj->id, addr, BusUpdt);
            if(Bus::read_ex == true) {
                obj->cache.cache_set_status(addr, Modified);
            } else {
                Protocol::cache_transfers++;
                obj->cache.cache_set_status(addr, ShModified);
            }
            pthread_mutex_unlock(&obj->lock);
            pthread_mutex_unlock(&Bus::req_lock);
            break;
        case ShModified:
            assert(op == "W");
            obj->cache.cache_incr_counter(addr);
            obj->cache.update_cache_lru(addr);
            obj->opt = BusUpdt;
            Bus::wait_for_responses(obj->id, addr, BusUpdt);
            if(Bus::read_ex == true) {
                obj->cache.cache_set_status(addr, Modified);
            } else {
                Protocol::cache_transfers++;
            }
            pthread_mutex_unlock(&obj->lock);
            pthread_mutex_unlock(&Bus::req_lock);
            break;

        case Invalid:
            if(op == "R") {
                obj->opt = BusRd;
                Bus::wait_for_responses(obj->id, addr, BusRd);

                if(Bus::read_ex == true) {
                    obj->cache.insert_cache(addr, Exclusive);
                } else {
                    obj->cache.insert_cache(addr, ShClean);
                }

                if(Bus::owner_id != -1) {
                    Protocol::cache_transfers++;
                    pthread_mutex_unlock(&obj->lock);
                    pthread_mutex_unlock(&Bus::req_lock);
                } else {
                    obj->pending_addr = addr;
                    pthread_mutex_unlock(&obj->lock);
                    pthread_mutex_unlock(&Bus::req_lock);
                    Memory::request(addr);
                    pthread_mutex_lock(&obj->lock);
                    obj->pending_addr = 0;
                    pthread_mutex_unlock(&obj->lock);
                }
            } else {
                obj->opt = BusRd;
                Bus::wait_for_responses(obj->id, addr, BusRd);
                bool read_ex = Bus::read_ex;

                if(Bus::owner_id != -1) {
                    Protocol::cache_transfers++;
                    pthread_mutex_unlock(&obj->lock);
                    pthread_mutex_unlock(&Bus::req_lock);

                } else {
                    obj->pending_addr = addr;
                    pthread_mutex_unlock(&obj->lock);
                    pthread_mutex_unlock(&Bus::req_lock);
                    Memory::request(addr);
                    pthread_mutex_lock(&obj->lock);
                    obj->pending_addr = 0;
                    pthread_mutex_unlock(&obj->lock);
                }

                if(read_ex == true) {
                    pthread_mutex_lock(&obj->lock);
                    obj->cache.insert_cache(addr, Modified);
                    pthread_mutex_unlock(&obj->lock);
                } else {
                    pthread_mutex_lock(&Bus::req_lock);
                    pthread_mutex_lock(&obj->lock);
                    obj->cache.insert_cache(addr, ShModified);
                    Protocol::bus_transactions++;

                    obj->opt = BusUpdt;
                    Bus::wait_for_responses(obj->id, addr, BusUpdt);

                    pthread_mutex_unlock(&obj->lock);
                    pthread_mutex_unlock(&Bus::req_lock);
                }
            }
            break;

        case Exclusive:
        case Modified:
        default:
            assert(0);

    }
}
