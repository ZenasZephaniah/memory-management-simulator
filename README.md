# Hardware-Memory-Simulator

A modular, hardware-level memory management simulator implemented in modern C++ (C++17). This project simulates the complete hardware memory transaction pipeline: translating virtual addresses via Process-Isolated Paging, filtering memory requests through a multi-level Set-Associative Cache hierarchy (L1/L2), and performing physical memory allocations using dynamic block allocation and coalescing strategies.

## System Architecture & Operations Flow

Every memory access in the simulator flows through the following pipeline:

$$\text{Virtual Address (Process Scope)} \longrightarrow \text{Paging/Translation Table} \longrightarrow \text{Physical Address} \longrightarrow \text{L1 Cache} \longrightarrow \text{L2 Cache} \longrightarrow \text{Physical Memory}$$

1. **Virtual Memory (Paging)**: Translates process-scoped virtual addresses to physical addresses. Supports multiple distinct processes simultaneously (process isolation) and handles page faults using page eviction policies (FIFO/LRU).
2. **Multilevel Cache Hierarchy (L1/L2)**: Simulates set-associative caches. On an L1 cache miss, L2 is queried. On an L2 miss, the transaction falls back to Main Memory, with retrieved blocks propagating back up through L2 and L1.
3. **Physical Memory (Allocator)**: Simulates physical RAM blocks using dynamic allocation strategies with 8-byte word alignment and automatic block coalescing on deallocation to minimize fragmentation.

---

## Implemented Features

### 1. Physical Memory Allocator
*   **Dynamic Algorithms**: Supports **First-Fit**, **Best-Fit**, and **Worst-Fit** memory search strategies.
*   **Contiguous Block Management**: Explicitly maintains a doubly-linked list of free and allocated memory segments.
*   **8-Byte Alignment Boundary**: Automatically rounds requested sizes up to the nearest multiple of 8 to mimic real physical RAM hardware structures.
*   **Coalescing Engine**: Automatically detects and merges adjacent free blocks recursively upon deallocation to keep external fragmentation low.

### 2. Multi-Level Set-Associative Cache Simulation
*   **Configurable Associativity**: Supports direct-mapped, $N$-way set-associative, and fully associative structures.
*   **Replacement Policies**: Fully models **FIFO** (First-In, First-Out), **LRU** (Least Recently Used), and **LFU** (Least Frequently Used).
*   **LFU-LRU Tie-Breaker**: Implements a hybrid policy where LFU frequency ties are broken by evicting the least recently used block.

### 3. Virtual Memory Paging System
*   **Multi-Process Isolation**: Isolates process address spaces using packed page keys: $(\text{Process ID} \ll 16) \mid \text{Virtual Page Number}$.
*   **Fault Eviction Handling**: Page faults dynamically evict victim frames using configured page replacement policies (FIFO/LRU). Marks dirty bits and logs simulated write-back actions to disk.

---

## Building and Compiling

The project uses a standard `Makefile` configuration and has no external dependencies.

```bash
# Clean previous builds and compile
make clean && make

# Launch the simulator interactively
./memsim
```

---

## Interactive Command Line Interface (CLI)

The simulator includes a real-time command loop. Below is a list of valid operations:

| Command | Description | Example |
| :--- | :--- | :--- |
| `init memory <size>` | Allocates a contiguous block of main physical memory | `init memory 1024` |
| `set allocator <strategy>` | Configures allocation model (`first`, `best`, or `worst`) | `set allocator best` |
| `setup cache <l1_sz> <l2_sz>`| Configures L1 and L2 cache capacities (LRU-based) | `setup cache 64 256` |
| `setup vm <page_size>` | Enables VM Paging with a fixed page boundary | `setup vm 256` |
| `malloc <bytes>` | Simulates memory allocation | `malloc 100` |
| `free <block_id>` | Frees and merges the specified block | `free 1` |
| `access <pid> <vaddr> <r\|w>` | Translates virtual address, runs cache check, and accesses data | `access 1 0x0050 r` |
| `dump` | Visual representation of page tables, cache sets, and physical blocks | `dump` |
| `stats` | Computes hit ratios, page fault rates, and fragmentation | `stats` |
| `exit` | Exits the simulator | `exit` |

---

## Running Automated Workloads

You can execute a predefined sequence of memory operations (trace) using input redirection:

```bash
# Run automated scenario tests
./memsim < tests/workload.txt > tests/output.log

# Check the results
cat tests/output.log
```
EOF
```

---

### Step 4: Final Compilation and Execution Check

Recompile and execute the clean, polished binary:

```bash
make clean && make && ./memsim
```

Verify that the interactive CLI runs correctly and that the `SYSTEM PAGE TABLE DUMP` is printed with clean spacing. 
