#ifndef VIRTUAL_MEMORY_HPP
#define VIRTUAL_MEMORY_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

using addr_t = uint32_t;

enum class PageReplacementPolicy {
    FIFO,
    LRU
};

// Represents a Page Table Entry (PTE)
struct PageTableEntry {
    uint32_t frame_number;
    bool valid;
    bool dirty;
    uint64_t last_accessed;  // Timestamp for LRU
    uint64_t insertion_time; // Timestamp for FIFO

    PageTableEntry() : frame_number(0), valid(false), dirty(false), last_accessed(0), insertion_time(0) {}
};

struct VMStats {
    uint64_t page_faults;
    uint64_t page_hits;
    double page_fault_rate;
};

class VirtualMemorySystem {
private:
    size_t page_size;
    size_t physical_memory_size;
    size_t num_frames;
    PageReplacementPolicy policy;

    uint64_t global_timer;
    uint64_t page_fault_count;
    uint64_t page_hit_count;

    // Page Table: Maps Page Number (VPN) -> Page Table Entry (PTE)
    std::unordered_map<uint32_t, PageTableEntry> page_table;

    // Frame Map: Tracks which virtual page occupies each physical frame
    // Frame Index -> Virtual Page Number (VPN). -1 represents an empty frame.
    std::vector<int32_t> frame_table;

    // Helper functions
    int32_t find_free_frame();
    uint32_t select_victim_page();

public:
    VirtualMemorySystem(size_t pg_size, size_t phys_mem_sz, PageReplacementPolicy pol);
    ~VirtualMemorySystem() = default;

    // Translates a Virtual Address to a Physical Address.
    // Triggers a Page Fault and Page Replacement if not present in memory.
    // Returns the translated Physical Address.
    addr_t translate(uint32_t process_id, addr_t virtual_address, bool is_write);

    void reset_stats();
    VMStats get_statistics() const;
    void print_statistics() const;
    void dump_page_table() const;
};

#endif // VIRTUAL_MEMORY_HPP
