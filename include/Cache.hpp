#ifndef CACHE_HPP
#define CACHE_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

using addr_t = uint32_t;

enum class ReplacementPolicy {
    FIFO,
    LRU,
    LFU
};

// Represents a single line (or block) in a cache set
struct CacheLine {
    addr_t tag;
    bool valid;
    bool dirty;
    uint64_t last_accessed; // Timestamp for LRU
    uint64_t access_count;  // Counter for LFU
    uint64_t insertion_time; // Counter for FIFO

    CacheLine() : tag(0), valid(false), dirty(false), last_accessed(0), access_count(0), insertion_time(0) {}
};

// Represents a set of cache lines (1 line for Direct-Mapped, N lines for N-way associative)
struct CacheSet {
    std::vector<CacheLine> lines;
};

struct CacheStats {
    uint64_t hits;
    uint64_t misses;
    double hit_rate;
};

class CacheLevel {
private:
    std::string level_name; // e.g., "L1" or "L2"
    size_t cache_size;      // Total size in bytes
    size_t block_size;      // Size of each block in bytes
    size_t associativity;   // 1 for Direct-Mapped, N for N-way Set Associative
    ReplacementPolicy policy;

    size_t num_sets;
    size_t index_bits;
    size_t offset_bits;

    uint64_t global_timer; // Simulated clock cycles to track LRU, FIFO, LFU metadata
    uint64_t hit_count;
    uint64_t miss_count;

    std::vector<CacheSet> sets;

    // Helper functions for bit-manipulation
    void calculate_parameters();
    addr_t get_index(addr_t address) const;
    addr_t get_tag(addr_t address) const;

public:
    CacheLevel(std::string name, size_t size, size_t block_sz, size_t assoc, ReplacementPolicy pol);
    ~CacheLevel() = default;

    // Checks cache. Returns true if hit, false if miss.
    bool access(addr_t address, bool is_write);

    // Inserts an address into the cache on a miss, handles eviction if the set is full.
    // Returns the evicted address if it was dirty, otherwise returns 0 (no write-back).
    addr_t insert(addr_t address, bool is_write);

    void reset_stats();
    CacheStats get_statistics() const;
    void print_statistics() const;
    void dump_cache() const;
};

#endif // CACHE_HPP
