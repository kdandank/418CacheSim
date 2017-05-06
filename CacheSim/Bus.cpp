#include "Bus.h"

pthread_mutex_t Bus::req_lock;
pthread_mutex_t Bus::resp_lock;
pthread_cond_t  Bus::req_cvar;
pthread_cond_t  Bus::resp_cvar;
int Bus::resp_count;
int Bus::owner_id;

operations Bus::opt;

void Bus::init() {
    pthread_mutex_init(&req_lock, NULL);
    pthread_mutex_init(&resp_lock, NULL);
    pthread_cond_init(&req_cvar, NULL);
    pthread_cond_init(&resp_cvar, NULL);
    resp_count = 0;
    owner_id = 0;
    opt = BusRdx;
}
