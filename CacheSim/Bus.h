#ifndef _BUS_H_
#define _BUS_H_

#include <pthread.h>

enum operations {
        BusRd = 0,
        BusRdx,
        BusWr,
        BusWrx,
        BusUpd
};

class Bus {

public:
    static pthread_mutex_t lock;
    static pthread_cond_t  cvar;
    static int resp_count;
    static int owner_id;

    static operations opt;

    static void init();
};

#endif /* _BUS_H_ */
