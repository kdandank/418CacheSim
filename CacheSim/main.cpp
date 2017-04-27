#include <iostream.h>

/**
 * Prints the usage for the program
 */
void print_usage() {
    std::cout << "./cache_sim [-c <Cache_size in MB>] [-a <Set Associativity>]"
                    " [-p <Cache Coherence Protocol>] [-t <Trace File>]\n";
}

void process_trace_file(std::string trace_filename) {
    ifstream trace_file(trace_filename);
    if(!trace_file) {
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

int main(int argc, char *argv[]) {
    int ch;
    int cache_size = 128;
    int associativity = 4;
    std::string protocol = "MSI";
    std::string trace_file = "trace.out";

    /* Read arguments from command line */
    while((ch = getopt(argc, argv, "c:a:p:t:")) != -1) {
        switch(ch) {
            case 'c':
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

    process_trace_file(trace_file);
}
