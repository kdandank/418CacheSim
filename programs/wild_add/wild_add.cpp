#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

long global_val;
//pthread_mutex_t lock;

void *wild_add(void *args) {

    long count = (long)args;
    for(long i = 0; i < count; i++) {
        //pthread_mutex_lock(&lock);
        global_val += 1;
        //pthread_mutex_unlock(&lock);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    if(argc != 3) {
        printf("\nUsage :: ./wild_add <num_threads> <num_iterations>");
        exit(-2);
    }

    //pthread_mutex_init(&lock, NULL);

    int rc;
    long t;

    long num_threads = atoi(argv[1]);
    long num_iters = atoi(argv[2]);
    
    pthread_t threads[num_threads];

    for(int t = 0; t < num_threads; t++){
        rc = pthread_create(&threads[t], NULL, wild_add, (void *)num_iters);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    /* Last thing that main() should do */
    pthread_exit(NULL);
    return 0;
}
