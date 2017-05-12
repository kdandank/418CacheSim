/**
 * Implementation specific to the MOSI protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _MOSI_H_
#define _MOSI_H_

class MOSI : public SnoopingCache {
public:
    MOSI(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(MOSI *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _MOSI_H_ */
