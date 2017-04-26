#include <iostream>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

volatile int number = 0;
volatile int target;

void dummy_instr_start() {
}

void dummy_instr_end() {
}

void *wait_target(void *arg) {
    while(number != target);

    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        cout << "Usage : " << argv[0] << " [NUM_THREADS] [NUM_TARGET]\n";
        return -1;
    }

    int num_threads = atoi(argv[1]);
    target = atol(argv[2]);

    pthread_t *tids = new pthread_t[num_threads];

    dummy_instr_start();
    for(int i = 1; i < num_threads; i++) {
        pthread_create(&tids[i], NULL, wait_target, NULL);
    }

    while(number != target) {
        number++;
    }

    for(int i = 1; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }
    dummy_instr_end();

    return 0;
}
