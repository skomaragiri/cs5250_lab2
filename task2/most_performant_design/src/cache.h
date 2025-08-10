#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool valid;
    bool dirty;
    uint32_t tag;
    int lru_counter;
} CacheBlock;

typedef struct {
    int num_sets;
    int assoc;
    int block_size;
    CacheBlock **blocks;
} Cache;

// Init function
Cache* cache_create(int size, int assoc, int block_size);

// Access types
typedef enum { READ, WRITE } AccessType;

// Returns true on hit, false on miss
bool cache_access(Cache *cache, uint32_t address, AccessType type);

// Called each cycle to decrement stall counters
void cache_cycle();

// Expose stall counters
extern int icache_stall;
extern int dcache_stall;

#endif
