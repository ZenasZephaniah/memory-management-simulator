#include "Allocator.hpp"
#include <iomanip>
#include <algorithm>

static size_t align_size(size_t size) {
    return (size + 7) & ~static_cast<size_t>(7);
}

PhysicalAllocator::PhysicalAllocator(size_t size)
    : total_size(size), current_strategy(AllocatorType::FIRST_FIT), next_block_id(1),
      total_alloc_requests(0), successful_alloc_requests(0) {
    memory_map.emplace_back(-1, 0, total_size, true);
}

void PhysicalAllocator::set_strategy(AllocatorType strategy) {
    current_strategy = strategy;
}

std::string PhysicalAllocator::get_strategy_string() const {
    switch (current_strategy) {
        case AllocatorType::FIRST_FIT: return "FIRST_FIT";
        case AllocatorType::BEST_FIT:  return "BEST_FIT";
        case AllocatorType::WORST_FIT: return "WORST_FIT";
    }
    return "UNKNOWN";
}

bool PhysicalAllocator::split_block(std::list<Block>::iterator& it, size_t requested_size, int block_id) {
    if (it->size < requested_size) {
        return false;
    }

    size_t remaining_size = it->size - requested_size;
    it->id = block_id;
    it->is_free = false;
    
    if (remaining_size >= 8) {
        it->size = requested_size;
        memory_map.insert(std::next(it), Block(-1, it->start_addr + requested_size, remaining_size, true));
    }
    return true;
}

void PhysicalAllocator::coalesce() {
    if (memory_map.size() < 2) return;

    auto it = memory_map.begin();
    while (it != memory_map.end()) {
        auto next_it = std::next(it);
        if (next_it != memory_map.end()) {
            if (it->is_free && next_it->is_free) {
                it->size += next_it->size;
                memory_map.erase(next_it);
                continue;
            }
        }
        ++it;
    }
}

addr_t PhysicalAllocator::allocate(size_t size) {
    total_alloc_requests++;
    size_t aligned_size = align_size(size);
    auto target_it = memory_map.end();
    
    if (current_strategy == AllocatorType::FIRST_FIT) {
        for (auto it = memory_map.begin(); it != memory_map.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                target_it = it;
                break;
            }
        }
    } 
    else if (current_strategy == AllocatorType::BEST_FIT) {
        size_t min_excess = size_t(-1);
        for (auto it = memory_map.begin(); it != memory_map.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                size_t excess = it->size - aligned_size;
                if (excess < min_excess) {
                    min_excess = excess;
                    target_it = it;
                }
            }
        }
    } 
    else if (current_strategy == AllocatorType::WORST_FIT) {
        size_t max_excess = 0;
        bool found = false;
        for (auto it = memory_map.begin(); it != memory_map.end(); ++it) {
            if (it->is_free && it->size >= aligned_size) {
                size_t excess = it->size - aligned_size;
                if (!found || excess > max_excess) {
                    max_excess = excess;
                    target_it = it;
                    found = true;
                }
            }
        }
    }

    if (target_it != memory_map.end()) {
        int assigned_id = next_block_id++;
        addr_t allocated_addr = target_it->start_addr;
        if (split_block(target_it, aligned_size, assigned_id)) {
            successful_alloc_requests++;
            return allocated_addr;
        }
    }
    return static_cast<addr_t>(-1);
}

bool PhysicalAllocator::deallocate(int block_id) {
    for (auto& block : memory_map) {
        if (!block.is_free && block.id == block_id) {
            block.is_free = true;
            block.id = -1;
            coalesce();
            return true;
        }
    }
    return false;
}

void PhysicalAllocator::dump_memory() const {
    std::cout << "\n================= MEMORY MAP DUMP =================" << std::endl;
    for (const auto& block : memory_map) {
        addr_t end_addr = block.start_addr + block.size - 1;
        std::cout << "[0x" << std::setw(4) << std::setfill('0') << std::hex << block.start_addr 
                  << " - 0x" << std::setw(4) << std::setfill('0') << end_addr << "] ";
        if (block.is_free) {
            std::cout << "FREE (Size: " << std::dec << block.size << " bytes)\n";
        } else {
            std::cout << "USED (id=" << std::dec << block.id << ", Size: " << block.size << " bytes)\n";
        }
    }
    std::cout << "===================================================" << std::endl;
}

AllocatorStats PhysicalAllocator::get_statistics() const {
    AllocatorStats stats{};
    stats.total_memory = total_size;
    stats.used_memory = 0;
    stats.free_memory = 0;
    stats.successful_allocations = successful_alloc_requests;
    stats.failed_allocations = total_alloc_requests - successful_alloc_requests;

    size_t largest_free_block = 0;
    for (const auto& block : memory_map) {
        if (block.is_free) {
            stats.free_memory += block.size;
            if (block.size > largest_free_block) {
                largest_free_block = block.size;
            }
        } else {
            stats.used_memory += block.size;
        }
    }

    stats.internal_fragmentation = 0.0;
    if (stats.free_memory > 0) {
        stats.external_fragmentation = (1.0 - (static_cast<double>(largest_free_block) / stats.free_memory)) * 100.0;
    } else {
        stats.external_fragmentation = 0.0;
    }

    if (stats.total_memory > 0) {
        stats.utilization = (static_cast<double>(stats.used_memory) / stats.total_memory) * 100.0;
    } else {
        stats.utilization = 0.0;
    }
    return stats;
}

void PhysicalAllocator::print_statistics() const {
    AllocatorStats stats = get_statistics();
    std::cout << "\n================ ALLOCATOR STATISTICS ================" << std::endl;
    std::cout << std::dec;
    std::cout << "Strategy Model        : " << get_strategy_string() << "\n";
    std::cout << "Total Memory Capacity : " << stats.total_memory << " bytes\n";
    std::cout << "Used Memory           : " << stats.used_memory << " bytes\n";
    std::cout << "Free Memory           : " << stats.free_memory << " bytes\n";
    std::cout << "Memory Utilization    : " << std::fixed << std::setprecision(2) << stats.utilization << "%\n";
    std::cout << "External Fragmentation: " << stats.external_fragmentation << "%\n";
    std::cout << "Allocation Efficiency : Success Rate: " 
              << (total_alloc_requests > 0 ? (static_cast<double>(stats.successful_allocations) / total_alloc_requests) * 100.0 : 0.0)
              << "% (" << stats.successful_allocations << "/" << total_alloc_requests << ")\n";
    std::cout << "======================================================" << std::endl;
}
