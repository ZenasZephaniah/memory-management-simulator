#include "VirtualMemory.hpp"
#include <iomanip>
#include <algorithm>

VirtualMemorySystem::VirtualMemorySystem(size_t pg_size, size_t phys_mem_sz, PageReplacementPolicy pol)
    : page_size(pg_size), physical_memory_size(phys_mem_sz), policy(pol),
      global_timer(0), page_fault_count(0), page_hit_count(0) {
    
    num_frames = physical_memory_size / page_size;
    frame_table.resize(num_frames, -1); 
}

int32_t VirtualMemorySystem::find_free_frame() {
    for (size_t i = 0; i < num_frames; ++i) {
        if (frame_table[i] == -1) {
            return static_cast<int32_t>(i);
        }
    }
    return -1; 
}

uint32_t VirtualMemorySystem::select_victim_page() {
    uint32_t victim_key = 0;
    
    if (policy == PageReplacementPolicy::FIFO) {
        uint64_t oldest_insertion = uint64_t(-1);
        for (size_t i = 0; i < num_frames; ++i) {
            uint32_t page_key = frame_table[i];
            const auto& entry = page_table[page_key];
            if (entry.insertion_time < oldest_insertion) {
                oldest_insertion = entry.insertion_time;
                victim_key = page_key;
            }
        }
    } 
    else if (policy == PageReplacementPolicy::LRU) {
        uint64_t oldest_access = uint64_t(-1);
        for (size_t i = 0; i < num_frames; ++i) {
            uint32_t page_key = frame_table[i];
            const auto& entry = page_table[page_key];
            if (entry.last_accessed < oldest_access) {
                oldest_access = entry.last_accessed;
                victim_key = page_key;
            }
        }
    }

    return victim_key;
}

addr_t VirtualMemorySystem::translate(uint32_t process_id, addr_t virtual_address, bool is_write) {
    global_timer++;
    uint32_t vpn = virtual_address / page_size;
    uint32_t offset = virtual_address % page_size;
    
    uint32_t page_key = (process_id << 16) | vpn;

    if (page_table.count(page_key) > 0 && page_table[page_key].valid) {
        page_hit_count++;
        PageTableEntry& entry = page_table[page_key];
        entry.last_accessed = global_timer;
        if (is_write) {
            entry.dirty = true;
        }
        return (entry.frame_number * page_size) + offset;
    }

    page_fault_count++;
    std::cout << "[PAGE FAULT] Process " << process_id 
              << " accessed Virtual Address 0x" << std::hex << virtual_address 
              << " (Page " << std::dec << vpn << ") - Loading page..." << std::endl;

    int32_t assigned_frame = find_free_frame();

    if (assigned_frame != -1) {
        frame_table[assigned_frame] = page_key;
        
        PageTableEntry& entry = page_table[page_key];
        entry.frame_number = assigned_frame;
        entry.valid = true;
        entry.dirty = is_write;
        entry.last_accessed = global_timer;
        entry.insertion_time = global_timer;

        std::cout << " -> Allocated Free Physical Frame " << assigned_frame << std::endl;
    } 
    else {
        uint32_t victim_key = select_victim_page();
        PageTableEntry& victim_entry = page_table[victim_key];
        
        uint32_t victim_pid = victim_key >> 16;
        uint32_t victim_vpn = victim_key & 0xFFFF;
        assigned_frame = victim_entry.frame_number;

        std::cout << " -> Memory Full! Evicting Process " << victim_pid 
                  << " Page " << victim_vpn << " from Physical Frame " << assigned_frame;
        if (victim_entry.dirty) {
            std::cout << " (Page is DIRTY - Syncing to simulated disk)";
        }
        std::cout << std::endl;

        victim_entry.valid = false;
        frame_table[assigned_frame] = page_key;

        PageTableEntry& entry = page_table[page_key];
        entry.frame_number = assigned_frame;
        entry.valid = true;
        entry.dirty = is_write;
        entry.last_accessed = global_timer;
        entry.insertion_time = global_timer;
    }

    return (assigned_frame * page_size) + offset;
}

void VirtualMemorySystem::reset_stats() {
    page_fault_count = 0;
    page_hit_count = 0;
}

VMStats VirtualMemorySystem::get_statistics() const {
    VMStats stats{};
    stats.page_faults = page_fault_count;
    stats.page_hits = page_hit_count;
    uint64_t total = page_fault_count + page_hit_count;
    stats.page_fault_rate = (total > 0) ? (static_cast<double>(page_fault_count) / total) * 100.0 : 0.0;
    return stats;
}

void VirtualMemorySystem::print_statistics() const {
    std::cout << std::setfill(' ') << std::dec; // Reset sticky formatting
    VMStats stats = get_statistics();
    std::cout << "\n================ VIRTUAL MEMORY STATISTICS ================" << std::endl;
    std::cout << "Page Size             : " << page_size << " bytes\n";
    std::cout << "Total Physical Frames : " << num_frames << "\n";
    std::cout << "Replacement Policy    : " << (policy == PageReplacementPolicy::FIFO ? "FIFO" : "LRU") << "\n";
    std::cout << "Total Page Hits       : " << stats.page_hits << "\n";
    std::cout << "Total Page Faults     : " << stats.page_faults << "\n";
    std::cout << "Page Fault Rate       : " << std::fixed << std::setprecision(2) << stats.page_fault_rate << "%\n";
    std::cout << "===========================================================" << std::endl;
}

void VirtualMemorySystem::dump_page_table() const {
    std::cout << std::setfill(' ') << std::dec; // Reset sticky formatting
    std::cout << "\n================= SYSTEM PAGE TABLE DUMP =================" << std::endl;
    std::cout << std::left << std::setw(12) << "Process ID" 
              << std::setw(15) << "Virtual Page" 
              << std::setw(15) << "Phys Frame" 
              << std::setw(10) << "Status" 
              << std::setw(10) << "Dirty" << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;
    for (const auto& pair : page_table) {
        uint32_t process_id = pair.first >> 16;
        uint32_t vpn = pair.first & 0xFFFF;
        const auto& entry = pair.second;

        std::cout << std::left << std::setw(12) << process_id 
                  << std::setw(15) << vpn 
                  << std::setw(15);
        if (entry.valid) {
            std::cout << entry.frame_number;
        } else {
            std::cout << "-";
        }
        std::cout << std::setw(10) << (entry.valid ? "VALID" : "INVALID")
                  << std::setw(10) << (entry.dirty ? "YES" : "NO") << std::endl;
    }
    std::cout << "==========================================================" << std::endl;
}
