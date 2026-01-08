#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <string>


const int MEMORY_SIZE = 1024; 
const int CACHE_L1_SIZE = 64; 
const int CACHE_L2_SIZE = 256;
const int BLOCK_SIZE = 16;    


struct MemorySegment {
    int id;              
    size_t size;            
    size_t start_addr;      
    bool is_allocated;     
    
    MemorySegment* next;   
    MemorySegment* prev;    
};


struct SimulationState {
    MemorySegment* head;
    int total_time_cycles;  
};


void initialize_system();
void* sim_malloc(size_t size);
void sim_free(int block_id);
void dump_memory_map();
void access_cache_hierarchy(unsigned int address); 
void print_stats(); 

#endif