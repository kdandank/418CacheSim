/**
 * The main file for the Cache simulator
 *
 * Authors :-
 * Kshitiz Dange <kdange@andrew.cmu.edu>
 * Yash Tibrewal <ytibrewa@andrew.cmu.edu>
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include "Cache.h"
#include "Protocol.h"
#include "Bus.h"
#include "Memory.h"
#include <atomic>

/**
 * Prints the usage for the program
 */
void print_usage() {
    std::cout << "./cache_sim [-c <Number of cores>] [-s <Cache_size in MB>] [-a <Set Associativity>]"
                    " [-p <Cache Coherence Protocol>] [-t <Trace File>]\n";
}

/**
 * Processes the input trace file line by line
 */
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

    std::atomic<long> local_count(0);

    while(tracefile >> std::dec >> thread_id) {
        tracefile >> op;
        tracefile >> std::hex >> addr;
        Protocol::process_mem_access(thread_id, op, addr);
        local_count++;
    }

    while(local_count != Protocol::trace_count);
}

/**
 * Displays the results
 */
void print_mem_metrics(){

    std::cout << "\nTotal Acceses = " << Protocol::trace_count;
    std::cout << "\nBus Transactions = " << Protocol::bus_transactions;
    std::cout << "\nMemory Requests = " << Protocol::mem_reqs;
    std::cout << "\nMemory Write Backs = " << Protocol::mem_write_backs;
    std::cout << "\nCache Transfers = " << Protocol::cache_transfers << "\n";
}

int main(int argc, char *argv[]) {
    int ch;
    int num_cores = 1;
    int cache_size = 1;
    int associativity = 8;
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
