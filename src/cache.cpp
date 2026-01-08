#include "memory_system.h"

// --- CACHE STRUCTURES ---
struct CacheLine {
    bool valid;
    unsigned int tag;
};

// We simulate L1 and L2
std::vector<CacheLine> L1_cache;
std::vector<CacheLine> L2_cache;

bool is_cache_init = false;

void init_cache_subsystem() {
    if (is_cache_init) return;
    
    // Calculate number of lines
    int l1_lines = CACHE_L1_SIZE / BLOCK_SIZE; // 64 / 16 = 4 lines
    int l2_lines = CACHE_L2_SIZE / BLOCK_SIZE; // 256 / 16 = 16 lines
    
    L1_cache.resize(l1_lines, {false, 0});
    L2_cache.resize(l2_lines, {false, 0});
    
    is_cache_init = true;
    std::cout << "[CACHE] L1 (" << l1_lines << " lines) & L2 (" << l2_lines << " lines) Initialized.\n";
}

void access_cache_hierarchy(unsigned int address) {
    if (!is_cache_init) init_cache_subsystem();

    // STRICT BITWISE CALCULATION
    // Offset bits = log2(BLOCK_SIZE) = log2(16) = 4 bits
    // Index bits L1 = log2(4 lines) = 2 bits
    // Index bits L2 = log2(16 lines) = 4 bits
    
    int offset_bits = 4;
    int index_bits_l1 = 2; 
    int index_bits_l2 = 4;

    // L1 Parsing
    unsigned int offset_mask = (1 << offset_bits) - 1;
    unsigned int index_mask_l1 = (1 << index_bits_l1) - 1;
    
    unsigned int l1_index = (address >> offset_bits) & index_mask_l1;
    unsigned int l1_tag   = address >> (offset_bits + index_bits_l1);

    // L2 Parsing (Usually tag is different because index is wider)
    unsigned int index_mask_l2 = (1 << index_bits_l2) - 1;
    unsigned int l2_index = (address >> offset_bits) & index_mask_l2;
    unsigned int l2_tag   = address >> (offset_bits + index_bits_l2);

    std::cout << "   -> [CACHE ACCESS Addr:" << address << "]\n";
    
    // CHECK L1
    if (L1_cache[l1_index].valid && L1_cache[l1_index].tag == l1_tag) {
        std::cout << "      L1 HIT (Time: 1 cycle)\n";
        return; // Done
    } 
    
    // L1 MISS -> CHECK L2
    std::cout << "      L1 MISS -> Checking L2 (Penalty: +10 cycles)\n";
    
    if (L2_cache[l2_index].valid && L2_cache[l2_index].tag == l2_tag) {
        std::cout << "      L2 HIT (Total Time: 11 cycles)\n";
        // Fill L1
        L1_cache[l1_index].valid = true;
        L1_cache[l1_index].tag = l1_tag;
        return;
    }

    // L2 MISS -> MAIN MEMORY
    std::cout << "      L2 MISS -> Fetch RAM (Penalty: +100 cycles)\n";
    
    // Fill L2
    L2_cache[l2_index].valid = true;
    L2_cache[l2_index].tag = l2_tag;
    
    // Fill L1
    L1_cache[l1_index].valid = true;
    L1_cache[l1_index].tag = l1_tag;
}