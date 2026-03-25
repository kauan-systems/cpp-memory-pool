/**
 * @file stress_test.cpp
 * @brief High-performance benchmark tool to compare MemoryPool vs System Malloc.
 * * This tool simulates heavy multi-threaded contention to validate the 
 * scalability and speedup of the custom lock-free MemoryPool implementation.
 */

#include "allocator.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

/**
 * @brief Injects a compiler barrier to prevent Dead Code Elimination (DCE).
 * * In high-performance benchmarks, compilers may optimize away allocations if 
 * the memory is never "used". This inline assembly forces the compiler to 
 * treat the pointer 'p' as an active dependency.
 * * @param p Pointer to the allocated memory block.
 */
inline void escape(void* p) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(p) : "memory");
#endif
}

/**
 * @brief Simulation payload representing a typical data object.
 * A 128-byte size is used to bridge multiple cache lines and simulate 
 * realistic memory pressure.
 */
struct Payload {
    uint64_t data[16]; 
};

/** @brief Number of allocations performed before bulk deallocation. */
const int BATCH_SIZE = 1000;

/**
 * @brief Execution routine for MemoryPool performance testing.
 * * Simulates a real-world scenario where threads perform rapid 
 * allocation/deallocation cycles.
 * * @param pool Reference to the MemoryPool instance.
 * @param iterations Total number of operations to perform.
 */
void worker_pool(MemoryPool<Payload>& pool, size_t iterations) {
    Payload* batch[BATCH_SIZE];
    for (size_t i = 0; i < iterations / BATCH_SIZE; ++i) {
        // Allocation phase
        for (int j = 0; j < BATCH_SIZE; ++j) {
            batch[j] = pool.allocate();
            if (batch[j]) escape(batch[j]);
        }
        // Deallocation phase
        for (int j = 0; j < BATCH_SIZE; ++j) {
            pool.deallocate(batch[j]);
        }
    }
}

/**
 * @brief Execution routine for Standard Malloc performance testing.
 * * Serves as the baseline for performance comparison.
 * * @param iterations Total number of operations to perform.
 */
void worker_malloc(size_t iterations) {
    Payload* batch[BATCH_SIZE];
    for (size_t i = 0; i < iterations / BATCH_SIZE; ++i) {
        for (int j = 0; j < BATCH_SIZE; ++j) {
            batch[j] = static_cast<Payload*>(std::malloc(sizeof(Payload)));
            if (batch[j]) escape(batch[j]);
        }
        for (int j = 0; j < BATCH_SIZE; ++j) {
            std::free(batch[j]);
        }
    }
}

/**
 * @brief Entry point for the benchmarking suite.
 * * Orchestrates thread spawning, timing measurement, and result reporting.
 * * @return int Status code (0 for success).
 */
int main() {
    const unsigned int num_threads = std::thread::hardware_concurrency();
    const size_t iterations_per_thread = 5000000; 
    
    // Pre-allocate 4M blocks to ensure zero-latency availability during stress
    MemoryPool<Payload> pool(4000000); 

    std::cout << "========================================================" << std::endl;
    std::cout << "   MEMORY POOL HIGH-PERFORMANCE STRESS TEST             " << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "Hardware Threads : " << num_threads << std::endl;
    std::cout << "Ops per Thread   : 5,000,000" << std::endl;
    std::cout << "Payload Size     : " << sizeof(Payload) << " bytes" << std::endl;
    std::cout << "Running Benchmark...\n" << std::endl;

    // --- Phase 1: Custom Lock-Free Pool Benchmark ---
    auto start_pool = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::jthread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) 
            threads.emplace_back(worker_pool, std::ref(pool), iterations_per_thread);
    } 
    auto end_pool = std::chrono::high_resolution_clock::now();

    // --- Phase 2: Standard System Malloc Benchmark ---
    auto start_malloc = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::jthread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) 
            threads.emplace_back(worker_malloc, iterations_per_thread);
    }
    auto end_malloc = std::chrono::high_resolution_clock::now();

    // --- Phase 3: Analytics and Speedup Calculation ---
    std::chrono::duration<double> d_pool = end_pool - start_pool;
    std::chrono::duration<double> d_malloc = end_malloc - start_malloc;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "RESULTS:" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "Custom MemoryPool : " << d_pool.count() << " seconds" << std::endl;
    std::cout << "System Std Malloc : " << d_malloc.count() << " seconds" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "SPEEDUP           : " << (d_malloc.count() / d_pool.count()) << "x faster" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    return 0;
}
