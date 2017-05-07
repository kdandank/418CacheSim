#include <vector>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "Cache.h"

#define CACHE_LINE_SIZE 4096

unsigned int Cache::cache_lines;
unsigned int Cache::cache_size;
unsigned int Cache::associativity;
unsigned int Cache::block_bits;
unsigned long Cache::num_set;
unsigned int Cache::set_bits;
unsigned long Cache::set_mask;

CacheLine::CacheLine(unsigned int t, unsigned char s) {
    tag = t;
    status = s;
    lru_num = 0;
}

Set::Set(unsigned int ass) {
    current_lru = 0;
    for(int i = 0; i < ass; i++) {
        unsigned int t = 0;
        unsigned char s = 'I';
        cl.push_back(CacheLine(t, s));
    }
}

Cache::Cache() {
    //unsigned int line_size = cache_size / CACHE_LINE_SIZE;
    unsigned int num_sets = cache_size / associativity;

    // #######
    assert((num_sets & (num_sets - 1)) == 0);
    // #######

    for(int i = 0; i < num_sets; i++) {
        sets.push_back(Set(associativity));
    }
}

void Cache::cache_init(unsigned int size, unsigned int ass) {

    cache_size = size;
    associativity = ass;
    cache_lines = 0;
    block_bits = 12;
    num_set = cache_size / associativity; // MUST be a power of 2
    set_bits = log2(num_set) + 1;
    set_mask = (num_set - 1) << block_bits;
}

void Cache::update_cache_lru(unsigned long addr) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];
    bool found_line = false;

    for(CacheLine &c: s.cl) {
        if(c.tag == tag) {
            found_line = true;

            // Update the lru for set
            s.current_lru++;

            // Update the cache line
            c.lru_num = s.current_lru;
            break;
        }
    }

    if(!found_line) {
        // This shoud not happen
        assert(0);
    }
}

void Cache::insert_cache(unsigned long addr, unsigned char status) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];
    bool found_line = false;

    // First try to find an 'Invalid' line
    for(CacheLine &c : s.cl) {
        if(c.status == 'I') {
            // This line can be evicted
            found_line = true;

            // Update the lru for set
            s.current_lru++;

            // Write to new cache line
            c.tag = tag;
            c.status = status;
            c.lru_num = s.current_lru;

            break;

        } else if (c.tag == tag) {
            // It should not be in cache at this time
            assert(0);
        }
    }

    if(!found_line) {
        // Do LRU now
        CacheLine &evict = s.cl[0];
        unsigned int low = INT_MAX;
        for(CacheLine &c : s.cl) {
            if(c.lru_num < low) {
                low = c.lru_num;
                evict = c;
            }
        }

        // Update the lru for set
        s.current_lru++;

        // Write to new cache line
        evict.tag = tag;
        evict.status = status;
        evict.lru_num = s.current_lru;
    }

}

char Cache::cache_status(unsigned long addr) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];
    unsigned char status = 'I';

    for(CacheLine &c: s.cl) {
        if(c.tag == tag) {
            status = c.status;
            break;
        }
    }

    return status;
}
