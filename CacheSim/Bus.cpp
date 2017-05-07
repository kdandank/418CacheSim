#include "Bus.h"
#include "Protocol.h"

pthread_mutex_t Bus::req_lock;
pthread_mutex_t Bus::resp_lock;
pthread_cond_t  Bus::req_cvar;
pthread_cond_t  Bus::resp_cvar;
int Bus::resp_count;
int Bus::owner_id;
unsigned long Bus::addr;
std::vector<bool> Bus::pending_work;
bool Bus::recv_nak;

operations Bus::opt;

void Bus::init() {

    pthread_mutex_init(&req_lock, NULL);
    pthread_mutex_init(&resp_lock, NULL);
    pthread_cond_init(&req_cvar, NULL);
    pthread_cond_init(&resp_cvar, NULL);
    resp_count = 0;
    owner_id = 0;
    addr = 0;
    opt = BusRdX;
    recv_nak = false;

    for(int i = 0; i < Protocol::num_cores; i++) {
        pending_work.push_back(false);
    }
}

void Bus::wait_for_responses(unsigned long address, operations oper) {

    pthread_mutex_lock(&resp_lock);
    addr = address;
    opt = oper;
    recv_nak = false;

    for(int i = 0; i < Protocol::num_cores; i++) {
        pending_work[i] = true;
    }
    pthread_cond_signal(&resp_cvar);

    while(resp_count != Protocol::num_cores - 1) {
        pthread_cond_wait(&req_cvar, &resp_lock);
    }

    pthread_mutex_unlock(&resp_lock);
}
