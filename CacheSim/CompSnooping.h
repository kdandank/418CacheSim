/**
 * Implementation specific to the Competitive Snooping protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _COMP_SNOOPING_H_
#define _COMP_SNOOPING_H_

class CompSnooping : public SnoopingCache {
public:
    CompSnooping(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(CompSnooping *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _COMP_SNOOPING_H_ */
