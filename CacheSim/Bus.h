#ifndef _BUS_H_
#define _BUS_H_

#include <pthread.h>
#include <vector>

enum operations {
        BusRd = 0,
        BusRdX,
        BusUpg
};

class Bus {

public:
    static pthread_mutex_t req_lock;
    static pthread_mutex_t resp_lock;
    static pthread_cond_t  req_cvar;
    static pthread_cond_t  resp_cvar;
    static int resp_count;
    static int owner_id;
    static unsigned long addr;
    static std::vector<bool> pending_work;
    static bool recv_nak;

    static operations opt;

    static void init(int num_cores);
    static void wait_for_responses(int id, unsigned long address,
                                        operations oper);
};

#endif /* _BUS_H_ */
