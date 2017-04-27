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
        ~CacheLine();
};

class Set
{
    public:
        unsigned int current_lru;
        std::vector<CacheLine> cl;

        Set();
        Set(unsigned int ass);
        ~Set();
};


class Cache
{
    private:
        unsigned int cahce_lines;
        unsigned int cache_size; // in MB
        unsigned int associativity;
        std::vector<Set> sets;
    public:

        Cache();
        Cache(unsigned int size, unsigned int ass);
        ~Cache();

        void update_cache_lru(unsigned int addr);
        void insert_cache(unsigned int addr, unsigned char status);
        char cache_status(unsigned int addr);
};

#endif /* _CACHE_H_ */
