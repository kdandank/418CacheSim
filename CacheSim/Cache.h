#ifndef _CACHE_H_
#define _CACHE_H_

#include <vector>
#include <math.h>
#include <assert.h>

#define CACHE_LINE_SIZE 4096

enum cache_state {
    Modified = 0,
    Shared,
    Invalid,
    Exclusive,
    Owner,
    ShModified,
    ShClean
};

class CacheLine
{
    public:
        unsigned int tag;
        cache_state status;
        unsigned int lru_num;
        CacheLine();
        CacheLine(unsigned int t, cache_state s);
};

class Set
{
    public:
        unsigned int current_lru;
        std::vector<CacheLine> cl;

        Set(unsigned int ass);
};


class Cache
{
    private:
        static unsigned int cache_lines;
        static unsigned int cache_size; // in MB
        static unsigned int associativity;
        static unsigned long num_set;
        static unsigned int set_bits;
        static unsigned long set_mask;

        std::vector<Set> sets;
    public:
        static unsigned int block_bits;

        Cache();

        void update_cache_lru(unsigned long addr);
        void insert_cache(unsigned long addr, cache_state status);
        cache_state cache_check_status(unsigned long addr);
        void cache_set_status(unsigned long addr, cache_state status);

        static void cache_init(unsigned int size, unsigned int ass);
};

#endif /* _CACHE_H_ */
