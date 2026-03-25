# High-Performance Lock-Free Memory Pool (C++20)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ Standard: 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Platform: FreeBSD / Linux](https://img.shields.io/badge/Platform-FreeBSD%20%7C%20Linux-lightgrey.svg)](https://www.freebsd.org/)

A production-grade, ultra-low latency memory allocator designed for extreme multi-threaded contention. Inspired by the **FreeBSD UMA (Universal Memory Allocator)**, this project implements a dual-layer caching strategy to outperform the standard system heap by eliminating lock contention.

## 📈 Performance Benchmark

Validation conducted via `stress_test.cpp` simulating heavy multi-threaded pressure (5,000,000 operations per thread).

| Allocator | Execution Time | Speedup |
| :--- | :--- | :--- |
| **System `std::malloc`** | 0.898211s | Baseline |
| **Custom `MemoryPool`** | **0.200252s** | **4.48x Faster** |

> **Technical Note:** The allocator maintains deterministic **O(1)** timing even under high CPU core contention, where standard allocators typically suffer from kernel-level lock-sharding overhead and cache-line bouncing.

## ⚙️ Engineering Architecture

### 1. Dual-Layer Allocation Strategy
To achieve near-zero latency, the pool implements a hierarchical memory access pattern:
* **L1 - Thread Local Storage (TLS):** Each thread maintains a private cache of blocks. Allocations from L1 require **zero synchronization**, operating at CPU register speeds.
* **L2 - Global Lock-Free Stack:** When L1 is exhausted, the thread synchronizes with the global pool using a **Lock-Free MPMC (Multi-Producer Multi-Consumer)** stack.

### 2. ABA Problem Mitigation & DWCAS
The implementation solves the classic ABA race condition in lock-free structures using:
* **Tagged Pointers:** Each pointer is coupled with a 64-bit generation counter.
* **Double-Width CAS (`-mcx16`):** Utilizes the `CMPXCHG16B` instruction to atomically update both the pointer and the tag in a single operation, ensuring total memory integrity.

### 3. Hardware-Level Optimizations
* **Cache-Line Alignment:** All blocks are aligned via `alignas(16)` and `std::max_align_t` to prevent **False Sharing**.
* **Compiler Barriers:** Injected `asm volatile` barriers prevent **Dead Code Elimination (DCE)** during high-performance paths.
* **Memory Model:** Strict use of `std::memory_order_acquire` / `release` for optimal CPU pipeline utilization.

## 🛠️ Build & Run

Optimized for **FreeBSD** and **Linux** (x86_64). Requires a compiler with C++20 support and `mcx16` instructions.

```bash
# Clone the repository
git clone https://github.com/kauan-systems/cpp-memory-pool
cd cpp-memory-pool

# Run the industrial build and benchmark suite
chmod +x build.sh
./build.sh
```

## 📁 Project Structure

* `include/`: Core **MemoryPool** and **TaggedPointer** template definitions.
* `src/main.cpp`: Unit validation and alignment reporting.
* `src/stress_test.cpp`: High-concurrency performance benchmark.
* `build.sh`: Unified automation script for Release-mode compilation (`-O3 -march=native`).
```text
.
├── include/       # Header-only templates
├── src/           # Implementation and tests
├── build.sh       # Automation script
├── CMakeLists.txt # Build system configuration
└── README.md      # Documentation
```

## 🎯 Finished Milestones

- [x] **C++20 Concepts:** Type-safe constraints for memory-aligned types.
- [x] **ABA Mitigation:** Fully implemented via Tagged Pointers and DWCAS (`-mcx16`).
- [x] **L1 TLS Cache:** Zero-contention path for ultra-fast thread-local access.
- [x] **Deterministic O(1):** Constant time allocation/deallocation cycle.
- [x] **Industrial Build System:** Automated CMake pipeline with performance flags.

---
*Developed for high-frequency trading and low-latency system environments.*
