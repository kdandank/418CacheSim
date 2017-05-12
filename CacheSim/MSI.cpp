/**
 * Implementation specific to the MSI protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

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
    pthread_mutex_init(&lock, NULL);
    pthread_create(&tid, NULL, request_worker, (void *) this);
    pthread_create(&tid, NULL, response_worker, (void *) this);
}

/**
 * Implementation for the response worker thread for each cache.
 */
void *MSI::response_worker(void *arg) {
    MSI *obj = (MSI *) arg;
    pthread_mutex_lock(&Bus::resp_lock);
    while(true) {
        while(!Bus::pending_work[obj->id]) {
            pthread_cond_wait(&Bus::resp_cvar, &Bus::resp_lock);
        }
        pthread_mutex_unlock(&Bus::resp_lock);

        pthread_mutex_lock(&obj->lock);

        if(obj->pending_addr == Bus::addr && (obj->opt != BusRd || Bus::opt != BusRd)) {
            assert(obj->pending_addr);
            Bus::recv_nak = true;
        } else {
            cache_state status = obj->cache.cache_check_status(Bus::addr);
            switch(status) {
                case Modified:
                    if(Bus::opt == BusRd) {
                        obj->cache.cache_set_status(Bus::addr, Shared);
                    } else {
                        assert(Bus::opt == BusRdX);
                        obj->cache.cache_set_status(Bus::addr, Invalid);
                    }
                    /* Need to write back to memory */
                    Protocol::mem_write_backs++;

                    /* This will never be on a pending address because if it
                     * was pending, this would have been NAKd
                     */
                    Bus::owner_id = obj->id;
                    break;
                case Shared:
                    if(Bus::opt == BusRdX) {
                        obj->cache.cache_set_status(Bus::addr, Invalid);
                    }
                    /* It is possible that we don't have the data ourselves */
                    if(obj->pending_addr != Bus::addr) {
                        Bus::owner_id = obj->id;
                    }
                    break;
                case Invalid:
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

/**
 * Implementation for the request worker thread for each cache.
 */
void *MSI::request_worker(void *arg) {
    MSI *obj = (MSI *)arg;
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
void MSI::handle_request(MSI *obj, std::string op, unsigned long addr) {
    assert(addr);
    bool done = false;
    bool cache_transfer = false;

    pthread_mutex_lock(&obj->lock);
    cache_state status = obj->cache.cache_check_status(addr);
    if (status == Modified ||
            (status == Shared && op == "R")) {
        obj->cache.update_cache_lru(addr);
        pthread_mutex_unlock(&obj->lock);
        return;
    }
    pthread_mutex_unlock(&obj->lock);

    while(!done) {
        pthread_mutex_lock(&Bus::req_lock);
        pthread_mutex_lock(&obj->lock);
        status = obj->cache.cache_check_status(addr);
        done = true;

        Protocol::bus_transactions++;

        switch(status) {
            case Shared:
                assert(op == "W");
                obj->cache.update_cache_lru(addr);
                obj->opt = BusRdX;
                Bus::wait_for_responses(obj->id, addr, BusRdX);
                if(Bus::recv_nak) {
                    done = false;
                } else {
                    obj->cache.cache_set_status(addr, Modified);
                }
                break;
            case Invalid:
                if(op == "R") {
                    obj->opt = BusRd;
                    Bus::wait_for_responses(obj->id, addr, BusRd);
                    if(Bus::recv_nak) {
                        done = false;
                    } else {
                        obj->cache.insert_cache(addr, Shared);
                        obj->pending_addr = addr;
                    }
                } else {
                    obj->opt = BusRdX;
                    Bus::wait_for_responses(obj->id, addr, BusRdX);
                    if(Bus::recv_nak) {
                        done = false;
                    } else {
                        obj->cache.insert_cache(addr, Modified);
                        obj->pending_addr = addr;
                    }
                }
                break;
            default:
                assert(0);
        }
        /* Cache transfer will only matter if it was in Invalid state */
        if(Bus::owner_id != -1) {
            obj->pending_addr = 0;
            cache_transfer = true;
        }
        pthread_mutex_unlock(&obj->lock);
        pthread_mutex_unlock(&Bus::req_lock);

        if(status == Invalid && done) {
            if(cache_transfer) {
                Protocol::cache_transfers++;
            } else {
                Memory::request(addr);
                pthread_mutex_lock(&obj->lock);
                obj->pending_addr = 0;
                pthread_mutex_unlock(&obj->lock);
            }
        }

    }
}
