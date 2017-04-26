#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

volatile long global_val;

void dummy_instr_start() {

}

void dummy_instr_end() {

}

void *wild_add(void *args) {

    long count = (long)args;
    for(long i = 0; i < count; i++) {
        global_val += 1;
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if(argc != 3) {
        printf("\nUsage :: ./wild_add <num_threads> <num_iterations>");
        exit(-2);
    }

    int rc;

    long num_threads = atoi(argv[1]);
    long num_iters = atoi(argv[2]);

    pthread_t threads[num_threads];

    dummy_instr_start();

    for(int t = 0; t < num_threads - 1; t++){
        rc = pthread_create(&threads[t], NULL, wild_add, (void *)num_iters);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    wild_add((void *)num_iters);

    /* Last thing that main() should do */
    for(int i = 0; i < num_threads - 1; i++) {
        pthread_join(threads[i], NULL);
    }

    dummy_instr_end();

    return 0;
}
