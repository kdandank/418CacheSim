#ifndef _CACHE_H_
#define _CACHE_H_

#include <vector>
#include <math.h>
#include <assert.h>

#define CACHE_LINE_SIZE 4096

class CacheLine
{
    public:
        unsigned int tag;
        unsigned char status;
        unsigned int lru_num;
        CacheLine();
        CacheLine(unsigned int t, unsigned char s);
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
        static unsigned int block_bits;
        static unsigned long num_set;
        static unsigned int set_bits;
        static unsigned long set_mask;

        std::vector<Set> sets;
    public:

        Cache();

        void update_cache_lru(unsigned long addr);
        void insert_cache(unsigned long addr, unsigned char status);
        char cache_check_status(unsigned long addr);
        void cache_set_status(unsigned long addr, char status);

        static void cache_init(unsigned int size, unsigned int ass);
};

#endif /* _CACHE_H_ */
