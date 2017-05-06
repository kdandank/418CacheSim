#ifndef _BUS_H_
#define _BUS_H_

#include <pthread.h>

class Bus {

public:
    static pthread_mutex_t lock;
    static pthread_cond_t  cond_var;
    static int resp_count;
    static int owner_id;

    static enum operations {
        BusRd = 0,
        BusRdx,
        BusWr,
        BusWrx,
        BusUpd
    }opts;

    static void init();
};
