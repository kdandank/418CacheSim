#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include "Cache.h"
#include "Protocol.h"

/**
 * Prints the usage for the program
 */
void print_usage() {
    std::cout << "./cache_sim [-c <Cache_size in MB>] [-a <Set Associativity>]"
                    " [-p <Cache Coherence Protocol>] [-t <Trace File>]\n";
}

void process_trace_file(std::string trace_filename) {
    std::ifstream tracefile(trace_filename.c_str());
    if(!tracefile) {
        std::cerr << "Could not open file : " << trace_filename << "\n";
        print_usage();
        exit(1);
    }

    int thread_id;
    std::string op;
    unsigned long addr;

    while(tracefile >> thread_id) {
        tracefile >> op;
        tracefile >> std::hex >> addr;
    }
}

/**
 * Creates num_cores number of cache objects
 * @param num_cores The number of cores
 * @return A vector of the cache objects
 */
std::vector<Cache> create_cache_objects(int cache_size, int associativity,
                                            int num_cores) {
    std::vector<Cache> caches;
    for(int i = 0; i < num_cores; i++) {
        caches.push_back(Cache(cache_size, associativity));
    }
    return caches;
}

int main(int argc, char *argv[]) {
    int ch;
    int num_cores = 1;
    int cache_size = 128;
    int associativity = 4;
    std::string protocol = "MSI";
    std::string trace_file = "trace.out";

    /* Read arguments from command line */
    while((ch = getopt(argc, argv, "c:s:a:p:t:")) != -1) {
        switch(ch) {
            case 'c':
                num_cores = atoi(optarg);
                break;
            case 's':
                cache_size = atoi(optarg);
                break;
            case 'a':
                associativity = atoi(optarg);
                break;
            case 'p':
                protocol = std::string(optarg);
                break;
            case 't':
                trace_file = std::string(optarg);
                break;
            default:
                std::cerr << "Illegal arguments\n";
                print_usage();
                return 1;
        }
    }

    std::vector<Cache> caches = create_cache_objects(cache_size, associativity,
                                                        num_cores);
    Protocol::initialize(protocol, caches, num_cores);
    process_trace_file(trace_file);
}
