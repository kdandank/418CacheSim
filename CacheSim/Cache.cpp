#include <vector>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include "Cache.h"
#include "Protocol.h"

#define CACHE_LINE_SIZE (1 << (BLOCK_BITS))

unsigned long Cache::cache_lines;
unsigned long Cache::cache_size;
unsigned long Cache::associativity;
unsigned long Cache::block_bits;
unsigned long Cache::num_set;
unsigned long Cache::set_bits;
unsigned long Cache::set_mask;

CacheLine::CacheLine(unsigned long t, cache_state s) {
    tag = t;
    status = s;
    lru_num = 0;
}

Set::Set(unsigned long ass) {
    current_lru = 0;
    for(long i = 0; i < ass; i++) {
        unsigned long t = 0;
        cache_state s = Invalid;
        cl.push_back(CacheLine(t, s));
    }
}

Cache::Cache() {
    //unsigned long line_size = cache_size / CACHE_LINE_SIZE;
    unsigned long num_sets = cache_size / associativity;

    // #######
    assert((num_sets & (num_sets - 1)) == 0);
    // #######

    for(long i = 0; i < num_sets; i++) {
        sets.push_back(Set(associativity));
    }
}

void Cache::cache_init(unsigned long size, unsigned long ass) {

    cache_size = size * (1 << 6);
    associativity = ass;
    cache_lines = 0;
    block_bits = CACHE_BITS;
    num_set = cache_size / associativity; // MUST be a power of 2
    set_bits = log2(num_set);
    assert(!(num_set & (num_set - 1)));
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

void Cache::insert_cache(unsigned long addr, cache_state status) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];
    bool found_line = false;

    // First try to find an 'Invalid' line
    for(CacheLine &c : s.cl) {
        if(c.status == Invalid) {
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

        Protocol::mem_write_backs++;
        CacheLine &evict = s.cl[0];
        unsigned long low = s.cl[0].lru_num;
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

        /* On eviction check if status indicates modification */
        if(evict.status == Modified || evict.status == Owner ||
           evict.status == ShModified) {
            Protocol::mem_write_backs++;
        }
    }

}

cache_state Cache::cache_check_status(unsigned long addr) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];
    cache_state status = Invalid;

    for(CacheLine &c: s.cl) {
        if(c.tag == tag) {
            status = c.status;
            break;
        }
    }

    return status;
}

void Cache::cache_set_status(unsigned long addr, cache_state status) {

    unsigned long set = (addr & set_mask) >> block_bits;
    unsigned long tag_mask = ~((1 << (set_bits + block_bits)) - 1);
    unsigned long tag = (addr & tag_mask) >> (set_bits + block_bits);

    Set &s = sets[set];

    for(CacheLine &c: s.cl) {
        if(c.tag == tag) {
            c.status = status;
            break;
        }
    }
}
