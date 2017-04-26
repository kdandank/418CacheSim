#include <iostream>
#include <pthread.h>
#include <stdlib.h>

using namespace std;

int num_threads;
int num_buckets;
int num_numbers;

void dummy_instr_start() {
}

void dummy_instr_end() {
}

volatile int *buckets;
volatile int *numbers;

pthread_mutex_t *locks;

void *fill_buckets(void *arg) {
    pthread_t tid = (pthread_t) arg;
    int range_start = num_numbers / num_threads * tid;
    int range_end = (tid == num_threads - 1) ? num_numbers :
                        range_start + num_numbers / num_threads;

    for(int i = range_start; i < range_end; i++) {
        pthread_mutex_lock(&locks[numbers[i]]);
        buckets[numbers[i]]++;
        pthread_mutex_unlock(&locks[numbers[i]]);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 4) {
        cout << "Usage : " << argv[0] << " [NUM_THREADS] [NUM_BUCKETS] [NUM_NUMBERS]\n";
        return -1;
    }

    num_threads = atoi(argv[1]);
    num_buckets = atoi(argv[2]);
    num_numbers = atoi(argv[3]);

    buckets = new int[num_buckets];
    numbers = new int[num_numbers];

    pthread_t *tids = new pthread_t[num_threads];
    locks = new pthread_mutex_t[num_buckets];

    //cout << num_threads << " " << num_buckets << " " << num_numbers << "\n";

    /* Initialize locks */
    for(int i = 0; i < num_buckets; i++) {
        pthread_mutex_init(&locks[i], NULL);
    }

    /* Fill numbers with random numbers in num_buckets range */
    srand(time(0));
    for(int i = 0; i < num_numbers; i++) {
        numbers[i] = rand() % num_buckets;
    }

    dummy_instr_start();

    for(int i = 1; i < num_threads; i++) {
        pthread_create(&tids[i], NULL, fill_buckets, (void *) ((long) i ));
    }

    fill_buckets((void *) ((long) 0));

    for(int i = 1; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }

    dummy_instr_end();

    /*long sum = 0;
    for(int i = 0; i < num_buckets; i++) {
        sum += buckets[i];
        cout << buckets[i] << "\n";
    }
    cout << sum <<"\n";*/
    return 0;
}
