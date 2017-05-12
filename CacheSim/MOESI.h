/**
 * Implementation specific to the MOESI protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _MOESI_H_
#define _MOESI_H_

class MOESI : public SnoopingCache {
public:
    MOESI(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(MOESI *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _MOESI_H_ */
