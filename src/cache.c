#include "cache.h"
#include <stdlib.h>
#include <string.h>

int icache_stall = 0;
int dcache_stall = 0;

Cache* cache_create(int size, int assoc, int block_size) {
    Cache *c = malloc(sizeof(Cache));
    c->num_sets = size / (assoc * block_size);
    c->assoc = assoc;
    c->block_size = block_size;
    c->blocks = malloc(sizeof(CacheBlock*) * c->num_sets);
    for (int i = 0; i < c->num_sets; i++) {
        c->blocks[i] = malloc(sizeof(CacheBlock) * assoc);
        for (int j = 0; j < assoc; j++) {
            c->blocks[i][j].valid = false;
            c->blocks[i][j].dirty = false;
            c->blocks[i][j].tag = 0;
            c->blocks[i][j].lru_counter = j;
        }
    }
    return c;
}

bool cache_access(Cache *cache, uint32_t address, AccessType type) {
    uint32_t index = (address >> 5) & (cache->num_sets - 1);
    uint32_t tag = address >> (5 + __builtin_ctz(cache->num_sets));

    CacheBlock *set = cache->blocks[index];

    // Search for hit
    for (int i = 0; i < cache->assoc; i++) {
        if (set[i].valid && set[i].tag == tag) {
            // Update LRU
            int old = set[i].lru_counter;
            for (int j = 0; j < cache->assoc; j++)
                if (set[j].lru_counter < old)
                    set[j].lru_counter++;
            set[i].lru_counter = 0;
            // Mark dirty on write
            if (type == WRITE)
                set[i].dirty = true;
            return true;
        }
    }

    // Miss – find LRU block
    int lru_way = 0;
    for (int i = 0; i < cache->assoc; i++) {
        if (!set[i].valid) {
            lru_way = i;
            break;
        }
        if (set[i].lru_counter == cache->assoc - 1)
            lru_way = i;
    }

    // Replace block
    set[lru_way].valid = true;
    set[lru_way].tag = tag;
    set[lru_way].dirty = (type == WRITE);
    for (int j = 0; j < cache->assoc; j++)
        if (j != lru_way)
            set[j].lru_counter++;
    set[lru_way].lru_counter = 0;

    return false;
}

void cache_cycle() {
    if (icache_stall > 0) icache_stall--;
    if (dcache_stall > 0) dcache_stall--;
}
