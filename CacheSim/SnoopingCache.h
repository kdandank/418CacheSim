/**
 * The interface for the base class of all the protocols
 *
 * Authors :-
 * Kshitiz Dange <kdange@andrew.cmu.edu>
 * Yash Tibrewal <ytibrewa@andrew.cmu.edu>
 */

#ifndef _SNOOPING_CACHE_H_
#define _SNOOPING_CACHE_H_

#include "Cache.h"
#include <string>
#include <pthread.h>
#include "Bus.h"

class SnoopingCache {
protected:
    Cache cache;
    pthread_t req_thread;
    pthread_t resp_thread;
    pthread_mutex_t lock;
    int id;
    unsigned long pending_addr;
    operations opt;
};

#endif /* _SNOOPING_CACHE_H_ */
