#include "bloom_filter.h"
#include <stdlib.h>
#include <string.h>

uint64_t calc_hash(const char* str, uint64_t modulus, uint64_t seed) {
    uint64_t hash = 0;
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        hash = (hash * seed + str[i]) % modulus;
    }
    
    return hash;
}

void bloom_init(struct BloomFilter* bloom_filter, uint64_t set_size, hash_fn_t hash_fn, uint64_t hash_fn_count) {
    bloom_filter->set = (uint64_t*)calloc((set_size + 63) / 64, sizeof(uint64_t));
    bloom_filter->set_size = set_size;
    bloom_filter->hash_fn = hash_fn;
    bloom_filter->hash_fn_count = hash_fn_count;
}

void bloom_destroy(struct BloomFilter* bloom_filter) {
    free(bloom_filter->set);
    bloom_filter->set = NULL;
    bloom_filter->set_size = 0;
}

void bloom_insert(struct BloomFilter* bloom_filter, Key key) {
    for (uint64_t i = 0; i < bloom_filter->hash_fn_count; i++) {
        uint64_t hash = bloom_filter->hash_fn(key, bloom_filter->set_size, i);
        uint64_t index = hash / 64;
        uint64_t bit = hash % 64;
        bloom_filter->set[index] |= (1ULL << bit);
    }
}

bool bloom_check(struct BloomFilter* bloom_filter, Key key) {
    for (uint64_t i = 0; i < bloom_filter->hash_fn_count; i++) {
        uint64_t hash = bloom_filter->hash_fn(key, bloom_filter->set_size, i);
        uint64_t index = hash / 64;
        uint64_t bit = hash % 64;
        if ((bloom_filter->set[index] & (1ULL << bit)) == 0) {
            return false;
        }
    }
    return true;
}
