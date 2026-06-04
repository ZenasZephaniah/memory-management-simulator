#include "Simulator.hpp"

int main() {
    MemorySimulator simulator;
    
    // Launch interactive user shell loop
    simulator.run_cli();
    
    return 0;
}
