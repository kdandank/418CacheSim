#ifndef _MEMORY_H_
#define _BUS_H_

#include <pthread.h>
#include <list>

class MemRequest {
public:
    unsigned long addr;
    pthread_cond_t cv;
    bool done;
    int waiters; /* Required for original thread to wait */

    MemRequest(unsigned long a);
};


class Memory {
private:
    static pthread_mutex_t lock;
    static std::list<MemRequest> req_table;
    static pthread_cond_t threads_cv;
    static pthread_cond_t req_cv;

    static void *memory_worker(void *arg);

public:
    static void initialize();
    static void request(unsigned long addr);
};

#endif /* _MEMORY_H_ */
