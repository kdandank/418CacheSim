#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include "Cache.h"
#include "Protocol.h"
#include "Bus.h"
#include "Memory.h"

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
        Protocol::process_mem_access(thread_id, op, addr);
    }
}

/**
 * Creates num_cores number of cache objects
 * @param num_cores The number of cores
 * @return A vector of the cache objects
 */
/*std::vector<Cache> create_cache_objects(int cache_size, int associativity,
                                            int num_cores) {
    std::vector<Cache> caches;
    for(int i = 0; i < num_cores; i++) {
        caches.push_back(Cache(cache_size, associativity));
    }
    return caches;
}*/

void print_mem_metrics(){

    std::cout << "\nBus Transactions = " << Protocol::bus_transactions;
    std::cout << "\nMemory Requests = " << Protocol::mem_reqs;
    std::cout << "\nMemory Write Backs = " << Protocol::mem_write_backs;
    std::cout << "\nCache Transfers = " << Protocol::cache_transfers;
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

    Bus::init(num_cores);
    Memory::initialize();
    Protocol::initialize(protocol, num_cores, cache_size, associativity);
    process_trace_file(trace_file);

    print_mem_metrics();
}
