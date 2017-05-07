#ifndef _SNOOPING_CACHE_H_
#define _SNOOPING_CACHE_H_

#include "Cache.h"
#include <string>

class SnoopingCache {
protected:
    Cache cache;
    pthread_t req_thread;
    pthread_t resp_thread;
    pthread_mutex_t lock;
    int id;
};

#endif /* _SNOOPING_CACHE_H_ */
