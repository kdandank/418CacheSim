#include <string>
#include <vector>
#include "Cache.h"

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

class Protocol {
public:
    static pthread_mutex_t lock;
    static pthread_cond_t trace_cv; /* Signal mem acc processor to continue */
    static pthread_cond_t worker_cv; /* Signal workers for a new access */

protected:
    static Protocol obj;
    static bool ready;
    static int request_id;
    static std::string request_op;
    static unsigned long request_addr;
    static int num_cores;
    static std::vector<Cache> caches;
    std::vector<pthread_t> req_threads;
    std::vector<pthread_t> resp_threads;

public:
    /**
     * Initialize the Protocol
     */
    static void initialize(std::string protocol, std::vector<Cache> caches,
                            int num_cores);

    /**
     * Process access to memory
     */
    static void process_mem_access(int thread_id, std::string op,
                                unsigned long addr);
};

#endif /* _PROTOCOL_H_ */
