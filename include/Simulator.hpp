#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "Allocator.hpp"
#include "Cache.hpp"
#include "VirtualMemory.hpp"
#include <memory>

class MemorySimulator {
private:
    std::unique_ptr<PhysicalAllocator> physical_allocator;
    std::unique_ptr<CacheLevel> l1_cache;
    std::unique_ptr<CacheLevel> l2_cache;
    std::unique_ptr<VirtualMemorySystem> vm_system;

    size_t memory_size;
    bool has_virtual_memory;
    bool has_caches;

    // Track total memory operations
    uint64_t total_accesses;
    uint64_t l1_hits;
    uint64_t l2_hits;
    uint64_t memory_fallback_accesses;

public:
    MemorySimulator();
    ~MemorySimulator() = default;

    // Initialize components
    void init_memory(size_t size);
    void setup_allocator(AllocatorType strategy);
    void setup_caches(size_t l1_sz, size_t l2_sz, ReplacementPolicy policy);
    void setup_virtual_memory(size_t page_sz, PageReplacementPolicy policy);

    // Main interaction APIs
    addr_t simulate_malloc(size_t size);
    bool simulate_free(int block_id);
    void simulate_access(uint32_t process_id, addr_t virtual_addr, bool is_write);

    // Diagnostics
    void print_overall_stats() const;
    void dump_system_state() const;

    // Command-line interface loop
    void run_cli();
};

#endif // SIMULATOR_HPP
