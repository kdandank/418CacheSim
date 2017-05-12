/**
 * Implementation specific to the Dragon protocol
 *
 * Authors:
 *     Kshitiz Dange (KDANGE)
 *     Yash Tibrewal (YTIBREWA)
 */

#include "SnoopingCache.h"

#ifndef _DRAGON_H_
#define _DRAGON_H_

class Dragon : public SnoopingCache {
public:
    Dragon(int cache_id);

private:
    static void *request_worker(void *arg);
    static void *response_worker(void *arg);
    static void handle_request(Dragon *obj, std::string op,
                                    unsigned long addr);
};

#endif /* _DRAGON_H_ */
