#include "memory_system.h"



void translate_and_access(unsigned int virtual_address) {

    access_cache_hierarchy(virtual_address); 
}
