# High-Performance Memory Pool Allocator (C++20)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ Standard: 20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Platform: FreeBSD / Linux](https://img.shields.io/badge/Platform-FreeBSD%20%7C%20Linux-lightgrey.svg)](https://www.freebsd.org/)

A low-level memory management project designed to explore hardware-level optimization and concurrent data structures. This implementation features a **fixed-size block allocator** inspired by kernel-level architectures, specifically the **FreeBSD UMA (Universal Memory Allocator)**.

## 🚀 Why This Project?

Standard heap allocation (`malloc`/`new`) often introduces unpredictable latency due to fragmentation and global lock contention. This project addresses these challenges by providing:

* **O(1) Deterministic Timing:** Constant-time allocation and deallocation paths.
* **Cache-Line Awareness:** Optimized data locality to reduce CPU cache misses and TLB pressure.
* **Zero Metadata Overhead:** Implements an **Intrusive Free-List** pattern, storing control pointers within the free blocks themselves.
* **Kernel-Level Philosophy:** Focused on the "Construct-on-Allocate" mindset found in high-performance Unix kernels.

## 📈 Benchmarking & Validation

The project includes a robust `stress_test.cpp` suite that compares the custom pool against the standard system heap. 

The goal isn't just raw speed, but **latency stability**. In high-pressure scenarios, this allocator maintains predictable performance where standard allocators might suffer from heap exhaustion or fragmentation-induced slowdowns.

> **Note:** Performance gains vary based on thread contention and object size. Run the provided benchmark script to see results on your specific hardware.

## ⚙️ Core Engineering Principles

### 1. ABA Problem Mitigation (Ongoing)
To ensure safety in lock-free environments, the project explores **Tagged Pointer** strategies. This prevents the common ABA race condition during concurrent `pop` operations by coupling pointers with generation counters.

### 2. C++20 Memory Model
Utilizes `std::atomic` with explicit memory ordering (`acquire`/`release`) to ensure synchronization between cores with minimal pipeline stalls, avoiding the heavy overhead of traditional mutexes.

### 3. Pointer Arithmetic & Alignment
* **Manual Alignment:** Strict adherence to `alignof(T)` and `std::max_align_t`.
* **Zero-Copy Logic:** Direct memory segmentation using `char*` casting and pointer arithmetic for maximum efficiency.

## 📁 Project Structure

* `include/`: Header-only template definition of the `MemoryPool` engine.
* `src/`: Functional validation (`main.cpp`) and high-pressure stress tests (`stress_test.cpp`).
* `build.sh`: Unified script for automated compilation and testing.

## 🛠️ Build & Run

Optimized for **FreeBSD** and **Arch Linux** environments.

```bash
# Clone the repository
git clone [https://github.com/kauan-systems/cpp-memory-pool](https://github.com/kauan-systems/cpp-memory-pool)
cd cpp-memory-pool

# Run the automated build and benchmark suite
chmod +x build.sh
./build.sh
```

## 🎯 Roadmap

- [x] C++20 Concepts: Type-safe template constraints for memory-aligned types.

- [x] Intrusive Free-List: Zero-metadata overhead per block.

- [x] ABA Problem Mitigation: Tagged pointer / Versioning strategy implemented for concurrent safety.

- [ ] Lock-Free Atomic Core: Optimized multi-producer/multi-consumer (MPMC) scaling.

- [ ] Slab Growth: Dynamic slab allocation from OS via mmap/valloc (FreeBSD-style).

- [ ] NUMA-Awareness: Local-node allocation for multi-socket systems.
