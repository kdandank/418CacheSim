#include <string>
#include <vector>
#include "Cache.h"
#include "SnoopingCache.h"
#include <atomic>

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

class Protocol {
public:
    static pthread_mutex_t lock;
    static pthread_cond_t trace_cv; /* Signal mem acc processor to continue */
    static pthread_cond_t worker_cv; /* Signal workers for a new access */
    static bool ready;
    static int request_id;
    static std::string request_op;
    static unsigned long request_addr;
    static int num_cores;
    static std::atomic <long> bus_transactions;
    static std::atomic <long> mem_reqs;
    static std::atomic <long> mem_write_backs;
    static std::atomic <long> cache_transfers;

private:
    static std::vector<SnoopingCache *> sn_caches;

public:
    /**
     * Initialize the Protocol
     */
    static void initialize(std::string protocol, int num_cores,
                                int cache_size, int associativity);

    /**
     * Process access to memory
     */
    static void process_mem_access(int thread_id, std::string op,
                                    unsigned long addr);
};

#endif /* _PROTOCOL_H_ */
