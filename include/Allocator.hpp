#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <cstdint>

using addr_t = uint32_t;

enum class AllocatorType {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
};

struct Block {
    int id;             
    addr_t start_addr;  
    size_t size;        
    bool is_free;       

    Block(int id, addr_t start, size_t sz, bool free)
        : id(id), start_addr(start), size(sz), is_free(free) {}
};

struct AllocatorStats {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    double internal_fragmentation; 
    double external_fragmentation; 
    double utilization;            
    size_t successful_allocations;
    size_t failed_allocations;
};

class PhysicalAllocator {
private:
    size_t total_size;
    AllocatorType current_strategy;
    std::list<Block> memory_map;  
    int next_block_id;

    size_t total_alloc_requests;
    size_t successful_alloc_requests;

    bool split_block(std::list<Block>::iterator& it, size_t requested_size, int block_id);
    void coalesce();

public:
    PhysicalAllocator(size_t size);
    ~PhysicalAllocator() = default;

    void set_strategy(AllocatorType strategy);
    std::string get_strategy_string() const;

    addr_t allocate(size_t size);
    bool deallocate(int block_id);

    void dump_memory() const;
    AllocatorStats get_statistics() const;
    void print_statistics() const;
};

#endif // ALLOCATOR_HPP
