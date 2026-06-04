#include "Simulator.hpp"
#include <sstream>
#include <iomanip>

MemorySimulator::MemorySimulator()
    : memory_size(1024), has_virtual_memory(false), has_caches(false),
      total_accesses(0), l1_hits(0), l2_hits(0), memory_fallback_accesses(0) {}

void MemorySimulator::init_memory(size_t size) {
    memory_size = size;
    physical_allocator = std::make_unique<PhysicalAllocator>(size);
    has_virtual_memory = false;
    has_caches = false;
    total_accesses = 0;
    l1_hits = 0;
    l2_hits = 0;
    memory_fallback_accesses = 0;
    std::cout << "[SYSTEM] Initialized contiguous physical memory of " << size << " bytes." << std::endl;
}

void MemorySimulator::setup_allocator(AllocatorType strategy) {
    if (physical_allocator) {
        physical_allocator->set_strategy(strategy);
        std::cout << "[SYSTEM] Allocator strategy set to " << physical_allocator->get_strategy_string() << "." << std::endl;
    } else {
        std::cout << "[ERROR] Memory not initialized yet. Run 'init memory <size>' first." << std::endl;
    }
}

void MemorySimulator::setup_caches(size_t l1_sz, size_t l2_sz, ReplacementPolicy policy) {
    // Blocks are standard 16 bytes. Associativity is set to 2-way for standard mapping depth.
    l1_cache = std::make_unique<CacheLevel>("L1_CACHE", l1_sz, 16, 2, policy);
    l2_cache = std::make_unique<CacheLevel>("L2_CACHE", l2_sz, 16, 4, policy);
    has_caches = true;
    std::cout << "[SYSTEM] Caches Configured: L1 (" << l1_sz << "B, 2-way), L2 (" << l2_sz << "B, 4-way)." << std::endl;
}

void MemorySimulator::setup_virtual_memory(size_t page_sz, PageReplacementPolicy policy) {
    vm_system = std::make_unique<VirtualMemorySystem>(page_sz, memory_size, policy);
    has_virtual_memory = true;
    std::cout << "[SYSTEM] Paging Virtual Memory enabled. Page size: " << page_sz << " bytes." << std::endl;
}

addr_t MemorySimulator::simulate_malloc(size_t size) {
    if (!physical_allocator) {
        std::cout << "[ERROR] Initialize memory first." << std::endl;
        return static_cast<addr_t>(-1);
    }
    addr_t addr = physical_allocator->allocate(size);
    if (addr == static_cast<addr_t>(-1)) {
        std::cout << "[ALLOCATION FAILED] Could not satisfy request of " << size << " bytes." << std::endl;
    } else {
        std::cout << "[ALLOCATION SUCCESS] Allocated at address 0x" << std::hex << std::setfill('0') << std::setw(4) << addr 
                  << std::dec << " (aligned size)." << std::endl;
    }
    return addr;
}

bool MemorySimulator::simulate_free(int block_id) {
    if (!physical_allocator) return false;
    bool success = physical_allocator->deallocate(block_id);
    if (success) {
        std::cout << "[DEALLOCATION] Block ID " << block_id << " successfully freed and coalesced." << std::endl;
    } else {
        std::cout << "[ERROR] Block ID " << block_id << " not found." << std::endl;
    }
    return success;
}

void MemorySimulator::simulate_access(uint32_t process_id, addr_t address, bool is_write) {
    std::cout << std::setfill(' ') << std::dec;
    std::cout << "\n--- Memory Access: Proc " << process_id << " -> Address 0x" << std::hex << address 
              << (is_write ? " (WRITE)" : " (READ)") << " ---" << std::endl;

    addr_t physical_addr = address;

    // Phase 1: Virtual Memory Address Translation
    if (has_virtual_memory) {
        physical_addr = vm_system->translate(process_id, address, is_write);
        std::cout << "[VM STEP] Translated Virtual Address 0x" << std::hex << address 
                  << " -> Physical Address 0x" << physical_addr << std::dec << std::endl;
    }

    total_accesses++;

    // Phase 2: Cache Hierarchy Check
    if (has_caches) {
        std::cout << "[CACHE STEP] Querying L1 Cache..." << std::endl;
        if (l1_cache->access(physical_addr, is_write)) {
            std::cout << " -> L1 HIT!" << std::endl;
            l1_hits++;
        } else {
            std::cout << " -> L1 MISS! Querying L2 Cache..." << std::endl;
            if (l2_cache->access(physical_addr, is_write)) {
                std::cout << " -> L2 HIT! Loading into L1..." << std::endl;
                l2_hits++;
                l1_cache->insert(physical_addr, is_write);
            } else {
                std::cout << " -> L2 MISS! Fallback access to Main Physical Memory..." << std::endl;
                memory_fallback_accesses++;
                
                // Load block up the cache hierarchy
                addr_t evicted_l2 = l2_cache->insert(physical_addr, is_write);
                if (evicted_l2 != 0) {
                    std::cout << "    [EVICTION] L2 evicted a dirty block at physical 0x" << std::hex << evicted_l2 << std::dec << std::endl;
                }
                l1_cache->insert(physical_addr, is_write);
            }
        }
    } else {
        std::cout << "[MEMORY STEP] Directly accessed Main Physical Memory at 0x" << std::hex << physical_addr << std::dec << std::endl;
        memory_fallback_accesses++;
    }
}

void MemorySimulator::print_overall_stats() const {
    std::cout << std::setfill(' ') << std::dec;
    std::cout << "\n=======================================================" << std::endl;
    std::cout << "             UNIFIED SIMULATOR STATISTICS              " << std::endl;
    std::cout << "=======================================================" << std::endl;
    std::cout << "Total Sim Memory Accesses : " << total_accesses << "\n";
    if (has_caches) {
        std::cout << "L1 Cache Hits             : " << l1_hits << "\n";
        std::cout << "L2 Cache Hits             : " << l2_hits << "\n";
        double cache_efficiency = (total_accesses > 0) ? (static_cast<double>(l1_hits + l2_hits) / total_accesses) * 100.0 : 0.0;
        std::cout << "Overall Cache Hit Rate    : " << std::fixed << std::setprecision(2) << cache_efficiency << "%\n";
    }
    std::cout << "Physical Memory Fallbacks : " << memory_fallback_accesses << "\n";
    std::cout << "=======================================================" << std::endl;

    if (physical_allocator) physical_allocator->print_statistics();
    if (has_caches) {
        l1_cache->print_statistics();
        l2_cache->print_statistics();
    }
    if (has_virtual_memory) vm_system->print_statistics();
}

void MemorySimulator::dump_system_state() const {
    std::cout << std::setfill(' ') << std::dec;
    if (physical_allocator) physical_allocator->dump_memory();
    if (has_caches) {
        l1_cache->dump_cache();
        l2_cache->dump_cache();
    }
    if (has_virtual_memory) vm_system->dump_page_table();
}

void MemorySimulator::run_cli() {
    std::string line;
    std::cout << "\n=======================================================" << std::endl;
    std::cout << "         WELCOME TO THE HARDWARE MEMORY SIMULATOR       " << std::endl;
    std::cout << "   Enter commands to manipulate memory in real-time.   " << std::endl;
    std::cout << "   Type 'help' to see list of valid CLI operations.    " << std::endl;
    std::cout << "=======================================================" << std::endl;

    while (true) {
        std::cout << "\nmemsim> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "help") {
            std::cout << "Available Commands:\n"
                      << "  init memory <size>                 - Initialize contiguous physical memory size\n"
                      << "  set allocator <first|best|worst>   - Choose allocation algorithm\n"
                      << "  setup cache <l1_size> <l2_size>    - Enable L1 and L2 Caches\n"
                      << "  setup vm <page_size>               - Enable Paging Virtual Memory\n"
                      << "  malloc <bytes>                     - Request physical memory allocation\n"
                      << "  free <block_id>                    - Deallocate physical memory block\n"
                      << "  access <proc_id> <vaddr> <r|w>     - Read/Write an address (simulates entire pipeline)\n"
                      << "  dump                               - Visualization of memory, cache, and page tables\n"
                      << "  stats                              - Show comprehensive metrics\n"
                      << "  exit                               - Exit simulation\n";
        }
        else if (cmd == "init") {
            std::string sub;
            size_t sz;
            if (ss >> sub >> sz && sub == "memory") {
                init_memory(sz);
            } else {
                std::cout << "Usage: init memory <size>" << std::endl;
            }
        }
        else if (cmd == "set") {
            std::string sub, strategy;
            if (ss >> sub >> strategy && sub == "allocator") {
                if (strategy == "first") setup_allocator(AllocatorType::FIRST_FIT);
                else if (strategy == "best") setup_allocator(AllocatorType::BEST_FIT);
                else if (strategy == "worst") setup_allocator(AllocatorType::WORST_FIT);
                else std::cout << "Unknown strategy. Choose: first, best, worst" << std::endl;
            } else {
                std::cout << "Usage: set allocator <first|best|worst>" << std::endl;
            }
        }
        else if (cmd == "setup") {
            std::string type;
            ss >> type;
            if (type == "cache") {
                size_t l1, l2;
                if (ss >> l1 >> l2) {
                    setup_caches(l1, l2, ReplacementPolicy::LRU);
                } else {
                    std::cout << "Usage: setup cache <l1_size> <l2_size>" << std::endl;
                }
            } else if (type == "vm") {
                size_t pg;
                if (ss >> pg) {
                    setup_virtual_memory(pg, PageReplacementPolicy::FIFO);
                } else {
                    std::cout << "Usage: setup vm <page_size>" << std::endl;
                }
            } else {
                std::cout << "Usage: setup <cache|vm> ... " << std::endl;
            }
        }
        else if (cmd == "malloc") {
            size_t bytes;
            if (ss >> bytes) {
                simulate_malloc(bytes);
            } else {
                std::cout << "Usage: malloc <bytes>" << std::endl;
            }
        }
        else if (cmd == "free") {
            int id;
            if (ss >> id) {
                simulate_free(id);
            } else {
                std::cout << "Usage: free <block_id>" << std::endl;
            }
        }
        else if (cmd == "access") {
            uint32_t pid;
            addr_t vaddr;
            char mode;
            if (ss >> pid >> std::hex >> vaddr >> mode) {
                bool is_write = (mode == 'w' || mode == 'W');
                simulate_access(pid, vaddr, is_write);
            } else {
                std::cout << "Usage: access <proc_id> <vaddr_hex> <r|w>" << std::endl;
            }
        }
        else if (cmd == "dump") {
            dump_system_state();
        }
        else if (cmd == "stats") {
            print_overall_stats();
        }
        else {
            std::cout << "Unknown command. Type 'help' for support." << std::endl;
        }
    }
}
