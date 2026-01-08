#include "memory_system.h"

// NOTE: Virtual Memory is marked OPTIONAL in the design spec.
// This module passes addresses directly to the physical cache.

void translate_and_access(unsigned int virtual_address) {
    // In a full implementation, we would check Page Tables here.
    // For this MVP, we assume Virtual Address = Physical Address (Identity Mapping).
    
    // Pass directly to cache hierarchy
    access_cache_hierarchy(virtual_address); 
}