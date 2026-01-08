# Memory Management Simulator (MMS)

## Overview
This project is a C++ simulation of an Operating System's memory management unit (MMU). It models the interaction between dynamic memory allocation strategies, physical memory fragmentation, and a multilevel CPU cache hierarchy.

The simulator provides an interactive shell for users to allocate and free memory, inspect the physical memory map, and view real-time fragmentation statistics.

## Features
* **Dynamic Memory Allocation:** Implements the **First Fit** strategy with automatic block splitting.
* **Memory Coalescing:** Automatically merges adjacent free blocks upon deallocation to reduce external fragmentation.
* **Multilevel Cache Simulation:**
    * **L1 Cache:** 64 Bytes (4 Lines), Direct Mapped.
    * **L2 Cache:** 256 Bytes (16 Lines), Direct Mapped, Inclusive.
    * Simulates cache hits, misses, and penalty cycles using strict bitwise operations.
* **Statistics Reporting:** Calculates and displays Memory Utilization and External Fragmentation percentages.
* **Virtual Memory:** Implements Identity Mapping (Virtual Address = Physical Address) for direct cache interaction.

## Project Structure
```text
memory-simulator/
├── src/                # Source code files
│   ├── allocator.cpp   # Memory allocation logic (malloc/free)
│   ├── cache.cpp       # L1/L2 Cache simulation
│   ├── main.cpp        # Interactive shell (CLI)
│   └── virtual_memory.cpp
├── include/            # Header files
│   └── memory_system.h
├── tests/              # Test artifacts
│   ├── workload.txt    # Sample input patterns
│   └── run_tests.sh    # Automated test script
├── docs/               # Documentation & Demo
│   ├── Design_Document.pdf
│   └── Test_Artifacts.pdf
└── Makefile            # Build configuration
```

## **How to Build and Run**
**Prerequisites**
* G++ Compiler (Supporting C++17)
* Make

**Build the Project**
Open the terminal in the project root and run:
```bash
make
```

**Run the Simulator**
Start the interactive shell:
```bash
./mms
```

**Usage**
Once inside the ```MMS_SHELL>```, you can use the following commands:
*  **Command**         **Description**
* ```malloc <size``` Allocates a block of ```size``` bytes.
* ```free <id>```    Frees the memory block with ID ```<id>```.
* ```dump```         Displays the current map of USed/Free memory blocks.
* ```stats```        Shows memory utilization and fragmentation metrics.
* ```exit```         Closes the simulator.

**Automated Testing**
To verify the system's correctness (Cache Hit logic and Coalescing), run the following
```bash
bash tests/run_tests.sh
```
**Expected Output:**
```
[PASS] Cache Hit Logic detected.
[PASS] Memory Coalescing detected.
```

## **Documentation**
Detailed design decisions,assumptions, and test logs can be found in the ```docs/``` directory.
