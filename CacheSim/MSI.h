/**
 * Implementation specific to the MSI protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _MSI_H_
#define _MSI_H_

class MSI : public SnoopingCache {
public:
    MSI(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(MSI *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _MSI_H_ */
