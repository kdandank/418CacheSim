#ifndef _MSI_H_
#define _MSI_H_

class MSI : Protocol {
public:
    MSI() ;

    void *request_worker(void *arg);

    void handle_request(int tid);
};

#endif /* _MSI_H_ */
