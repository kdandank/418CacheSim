#include "Bus.h"

pthread_mutex_t Bus::lock;
pthread_cond_t  Bus::cvar;
int Bus::resp_count;
int Bus::owner_id;

operations Bus::opt;

void Bus::init() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cvar, NULL);
    resp_count = 0;
    owner_id = 0;
    opt = BusRdx;
}
