#include "memory_system.h"

SimulationState sys_state;
int global_block_id = 1;

void initialize_system() {
    sys_state.head = new MemorySegment();
    sys_state.head->id = 0; 
    sys_state.head->size = MEMORY_SIZE;
    sys_state.head->start_addr = 0;
    sys_state.head->is_allocated = false;
    sys_state.head->next = nullptr;
    sys_state.head->prev = nullptr;
    sys_state.total_time_cycles = 0;

    std::cout << "[SYSTEM] Physical Memory Initialized: " << MEMORY_SIZE << " bytes.\n";
}

void* sim_malloc(size_t req_size) {
    MemorySegment* current = sys_state.head;

    while (current != nullptr) {
        if (!current->is_allocated && current->size >= req_size) {
 
            size_t original_size = current->size;
            size_t remaining_size = original_size - req_size;
            
            current->is_allocated = true;
            current->size = req_size;
            current->id = global_block_id++;

            if (remaining_size > 0) {
                MemorySegment* new_free_block = new MemorySegment();
                new_free_block->id = 0; 
                new_free_block->size = remaining_size;
                new_free_block->start_addr = current->start_addr + req_size;
                new_free_block->is_allocated = false;
                
                new_free_block->next = current->next;
                new_free_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_free_block;
                }
                current->next = new_free_block;
            }

            std::cout << "[ALLOC] ID:" << current->id << " allocated " << req_size << " bytes at Addr: " << current->start_addr << "\n";
            
            access_cache_hierarchy(current->start_addr);
            
            return (void*)current->start_addr;
        }
        current = current->next;
    }
    std::cout << "[ERROR] Allocation failed. Not enough contiguous memory.\n";
    return nullptr;
}

void sim_free(int block_id) {
    MemorySegment* current = sys_state.head;
    bool found = false;

    while (current != nullptr) {
        if (current->is_allocated && current->id == block_id) {
            current->is_allocated = false;
            current->id = 0; // Reset ID
            std::cout << "[FREE] Block freed at Addr: " << current->start_addr << "\n";
            found = true;

            if (current->next && !current->next->is_allocated) {
                MemorySegment* temp = current->next;
                current->size += temp->size;
                current->next = temp->next;
                if (current->next) current->next->prev = current;
                delete temp;
                std::cout << "       (Merged with right neighbor)\n";
            }

            if (current->prev && !current->prev->is_allocated) {
                MemorySegment* prev_block = current->prev;
                prev_block->size += current->size;
                prev_block->next = current->next;
                if (current->next) current->next->prev = prev_block;
                delete current;
                std::cout << "       (Merged with left neighbor)\n";
            }
            break;
        }
        current = current->next;
    }
    if (!found) std::cout << "[ERROR] Invalid Block ID: " << block_id << "\n";
}

void dump_memory_map() {
    std::cout << "\n--- PHYSICAL MEMORY MAP ---\n";
    MemorySegment* current = sys_state.head;
    while (current) {
        std::cout << "[" << (current->is_allocated ? "USED" : "FREE") << "] "
                  << "Addr: " << std::setw(4) << current->start_addr 
                  << " | Size: " << std::setw(4) << current->size 
                  << " | ID: " << current->id << "\n";
        current = current->next;
    }
    std::cout << "---------------------------\n";
}


void print_stats() {
    size_t total_used = 0;
    size_t total_free = 0;
    size_t largest_free_block = 0;
    int free_blocks_count = 0;
    int allocated_blocks_count = 0;

    MemorySegment* current = sys_state.head;
    while (current != nullptr) {
        if (current->is_allocated) {
            total_used += current->size;
            allocated_blocks_count++;
        } else {
            total_free += current->size;
            free_blocks_count++;
            if (current->size > largest_free_block) {
                largest_free_block = current->size;
            }
        }
        current = current->next;
    }

    double utilization = (total_used > 0) ? ((double)total_used / MEMORY_SIZE) * 100.0 : 0.0;
    
    double ext_frag = 0.0;
    if (total_free > 0) {
        ext_frag = (1.0 - ((double)largest_free_block / total_free)) * 100.0;
    }

    std::cout << "\n--- MEMORY STATISTICS ---\n";
    std::cout << "Total Memory:      " << MEMORY_SIZE << " bytes\n";
    std::cout << "Used Memory:       " << total_used << " bytes (" << std::fixed << std::setprecision(2) << utilization << "%)\n";
    std::cout << "Free Memory:       " << total_free << " bytes\n";
    std::cout << "Total Allocations: " << allocated_blocks_count << "\n";
    std::cout << "Free Blocks (Holes): " << free_blocks_count << "\n";
    std::cout << "Largest Free Block:  " << largest_free_block << " bytes\n";
    std::cout << "Internal Frag.:      0 bytes (Exact fit allocation)\n";
    std::cout << "External Frag.:      " << ext_frag << "%\n";
    std::cout << "-------------------------\n";
}