#include "Cache.hpp"
#include <iomanip>
#include <cmath>
#include <algorithm>

// Safe bitwise log2 helper to avoid floating point math
static size_t calculate_log2(size_t value) {
    size_t result = 0;
    while (value >>= 1) {
        result++;
    }
    return result;
}

CacheLevel::CacheLevel(std::string name, size_t size, size_t block_sz, size_t assoc, ReplacementPolicy pol)
    : level_name(name), cache_size(size), block_size(block_sz), associativity(assoc), policy(pol),
      global_timer(0), hit_count(0), miss_count(0) {
    
    calculate_parameters();
    
    // Initialize the cache sets
    sets.resize(num_sets);
    for (size_t i = 0; i < num_sets; ++i) {
        sets[i].lines.resize(associativity);
    }
}

void CacheLevel::calculate_parameters() {
    size_t total_blocks = cache_size / block_size;
    num_sets = total_blocks / associativity;
    
    if (num_sets == 0) {
        num_sets = 1; // Safeguard for fully-associative configurations
    }

    offset_bits = calculate_log2(block_size);
    index_bits = calculate_log2(num_sets);
}

addr_t CacheLevel::get_index(addr_t address) const {
    if (num_sets <= 1) return 0; // Fully associative maps everything to Set 0
    return (address >> offset_bits) & (num_sets - 1);
}

addr_t CacheLevel::get_tag(addr_t address) const {
    return address >> (offset_bits + index_bits);
}

bool CacheLevel::access(addr_t address, bool is_write) {
    global_timer++;
    addr_t index = get_index(address);
    addr_t tag = get_tag(address);

    CacheSet& target_set = sets[index];

    for (auto& line : target_set.lines) {
        if (line.valid && line.tag == tag) {
            // Hit!
            hit_count++;
            line.last_accessed = global_timer;
            line.access_count++;
            if (is_write) {
                line.dirty = true;
            }
            return true;
        }
    }

    // Miss
    miss_count++;
    return false;
}

addr_t CacheLevel::insert(addr_t address, bool is_write) {
    global_timer++;
    addr_t index = get_index(address);
    addr_t tag = get_tag(address);

    CacheSet& target_set = sets[index];

    // Case 1: Search for an empty/invalid line to occupy
    for (auto& line : target_set.lines) {
        if (!line.valid) {
            line.tag = tag;
            line.valid = true;
            line.dirty = is_write;
            line.last_accessed = global_timer;
            line.access_count = 1;
            line.insertion_time = global_timer;
            return 0; // Succesfully inserted without eviction
        }
    }

    // Case 2: The set is completely full. We must evict a victim line.
    size_t victim_idx = 0;

    if (policy == ReplacementPolicy::FIFO) {
        uint64_t min_insert = uint64_t(-1);
        for (size_t i = 0; i < target_set.lines.size(); ++i) {
            if (target_set.lines[i].insertion_time < min_insert) {
                min_insert = target_set.lines[i].insertion_time;
                victim_idx = i;
            }
        }
    } 
    else if (policy == ReplacementPolicy::LRU) {
        uint64_t min_access = uint64_t(-1);
        for (size_t i = 0; i < target_set.lines.size(); ++i) {
            if (target_set.lines[i].last_accessed < min_access) {
                min_access = target_set.lines[i].last_accessed;
                victim_idx = i;
            }
        }
    } 
    else if (policy == ReplacementPolicy::LFU) {
        uint64_t min_frequency = uint64_t(-1);
        uint64_t oldest_access = uint64_t(-1);

        for (size_t i = 0; i < target_set.lines.size(); ++i) {
            const auto& line = target_set.lines[i];
            if (line.access_count < min_frequency) {
                min_frequency = line.access_count;
                oldest_access = line.last_accessed;
                victim_idx = i;
            } 
            // LRU Tie-breaker for identical LFU frequencies
            else if (line.access_count == min_frequency) {
                if (line.last_accessed < oldest_access) {
                    oldest_access = line.last_accessed;
                    victim_idx = i;
                }
            }
        }
    }

    CacheLine& victim = target_set.lines[victim_idx];
    addr_t evicted_addr = 0;

    // If the victim is dirty, calculate its original memory physical address to write it back
    if (victim.dirty) {
        evicted_addr = (victim.tag << (offset_bits + index_bits)) | (index << offset_bits);
    }

    // Overwrite the victim block metadata with the incoming block information
    victim.tag = tag;
    victim.valid = true;
    victim.dirty = is_write;
    victim.last_accessed = global_timer;
    victim.access_count = 1;
    victim.insertion_time = global_timer;

    return evicted_addr; // Returns the address that needs to be written back to physical memory/lower cache
}

void CacheLevel::reset_stats() {
    hit_count = 0;
    miss_count = 0;
}

CacheStats CacheLevel::get_statistics() const {
    CacheStats stats{};
    stats.hits = hit_count;
    stats.misses = miss_count;
    uint64_t total = hit_count + miss_count;
    stats.hit_rate = (total > 0) ? (static_cast<double>(hit_count) / total) * 100.0 : 0.0;
    return stats;
}

void CacheLevel::print_statistics() const {
    CacheStats stats = get_statistics();
    std::cout << "\n================ CACHE " << level_name << " STATISTICS ================" << std::endl;
    std::cout << "Access Model          : " << associativity << "-way Set Associative\n";
    std::cout << "Replacement Policy    : ";
    if (policy == ReplacementPolicy::FIFO) std::cout << "FIFO\n";
    else if (policy == ReplacementPolicy::LRU) std::cout << "LRU\n";
    else if (policy == ReplacementPolicy::LFU) std::cout << "LFU\n";
    std::cout << "Total Cache Hits      : " << stats.hits << "\n";
    std::cout << "Total Cache Misses    : " << stats.misses << "\n";
    std::cout << "Cache Hit Rate        : " << std::fixed << std::setprecision(2) << stats.hit_rate << "%\n";
    std::cout << "=========================================================" << std::endl;
}

void CacheLevel::dump_cache() const {
    std::cout << "\n================= CACHE " << level_name << " STORAGE DUMP =================" << std::endl;
    for (size_t s = 0; s < num_sets; ++s) {
        std::cout << "Set " << std::dec << s << ": ";
        for (size_t l = 0; l < associativity; ++l) {
            const auto& line = sets[s].lines[l];
            std::cout << "[";
            if (line.valid) {
                std::cout << "V=1, Tag=0x" << std::hex << line.tag 
                          << ", D=" << (line.dirty ? "1" : "0")
                          << ", Freq=" << std::dec << line.access_count;
            } else {
                std::cout << "INVALID";
            }
            std::cout << "] ";
        }
        std::cout << "\n";
    }
    std::cout << "================================================================" << std::endl;
}
