# Low-Level Memory Pool Allocator (C++20)

A high-performance memory management tool developed to explore the principles of operating system internals and resource optimization. This project implements a fixed-size block allocator, similar to those found in kernel-level memory management (e.g., FreeBSD's UMA).

## 🚀 Why This Project?

Standard memory allocation (`new`/`malloc`) can be slow and lead to heap fragmentation. In systems programming, we often need:

* **O(1) Allocation/Deallocation:** Predictable, constant-time performance. By using a pre-allocated pool, we eliminate the overhead of searching the heap.
* **Data Locality:** Keeping related objects close in memory to improve CPU cache hits, reducing the time the processor spends waiting for data from RAM.
* **Resource Control:** Pre-allocating memory to prevent runtime exhaustion (OOM) in constrained environments and avoiding "swiss cheese" fragmentation.

## 📈 Performance Results (The Proof)

To validate the implementation, a stress test was conducted performing **10,000,000 allocations/deallocations** against the standard system heap (`std::malloc`).

| Allocator | Time (10M Ops) | Speedup |
| :--- | :--- | :--- |
| **Standard Malloc** | 0.114264s | 1.0x (Baseline) |
| **Custom Memory Pool** | **0.008737s** | **13.07x Faster** 🚀 |

**Verdict:** The custom pool achieves massive performance gains by ensuring **O(1) complexity** and superior **Cache Locality**.

## 🛠️ Technical Breakdown

### 1. Intrusive Free-List Pattern
To minimize overhead, this allocator uses an **intrusive linked list**. Instead of external tracking, it repurposes the memory within the free blocks to store pointers to the next available slot. This ensures **zero metadata overhead** for inactive blocks.

### 2. Memory Alignment & Padding
- **Automatic Sizing:** The allocator automatically calculates the `block_unit_size` to satisfy both the user's data type (`T`) and the internal control structure (`Block*`).
- **Pointer Arithmetic:** Manual byte-perfect offsets using `char*` casting to segment a large pre-allocated buffer into aligned cells.
- **Performance-First I/O:** Prioritizes `\n` over `std::endl` to minimize expensive system-level buffer flushes.

## 📁 Project Structure

* `include/`: Header-only template definition of the `MemoryPool` class.
* `src/`: Functional validation (`main.cpp`) and high-pressure benchmarks (`stress_test.cpp`).
* `CMakeLists.txt`: Build configuration with strict compiler flags (`-O3`, `-Wall`, `-Wextra`).
* `build.sh`: Unified script for automated compilation and testing.

## ⚙️ How to Build & Run

Built and tested on **Arch Linux** using **GCC 15.2.1**.

```bash
# Clone the repository
git clone https://github.com/kauan-systems/cpp-memory-pool
cd cpp-memory-pool.

# Run the automated build and test script
chmod +x build.sh
./build.sh
```

## 🎯 Roadmap

- [x] **Generic Template Support:** Enabled allocation for any data type using C++ templates.
- [x] **Benchmark Suite:** High-pressure test vs Standard Malloc.
- [x] **Thread-Safety:** Implement lock-free atomic pointers for multi-threaded environments.
- [ ] **Slab Growth:** Allow the pool to request more memory "slabs" from the OS dynamically.


Developed as part of my Software Engineering journey, focusing on Systems Programming and Kernel internals.
