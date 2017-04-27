#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

class Protocol {
public:
    static pthread_mutex_t lock;
    static pthread_cond_t trace_cv;
    static pthread_cond_t worker_cv;

private:
    static Protocol obj;
    static atomic<bool> ready = false;
    static int request_id;
    static std::string request_op;
    static unsigned long requeset_addr;
    static int num_cores;
    static std::vector<Cache> caches;
    std::vector<pthread_t> req_threads;
    std::vector<pthread_t> resp_threads;

    /**
     * Initialize the Protocol
     */
    static void initialize(std::string protocol, std::vector<Cache> caches,
                            int num_cores);

    /**
     * Process access to memory
     */
    static void process_mem_access(int thread_id, std::string op,
                                unsigned long addr);
};

#endif /* _PROTOCOL_H_ */
