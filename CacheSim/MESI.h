/**
 * Implementation specific to the MESI protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _MESI_H_
#define _MESI_H_

class MESI : public SnoopingCache {
public:
    MESI(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(MESI *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _MESI_H_ */
