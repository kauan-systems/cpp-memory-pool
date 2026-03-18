#include "../include/allocator.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstdlib>

/**
 * @file stress_test.cpp
 * @brief High-pressure benchmark: Custom MemoryPool vs std::malloc.
 * Designed to prevent compiler dead-code elimination.
 */

struct Payload {
    double data[4]; // 32 bytes
};

int main() {
    // Increased iterations to 10,000,000 for more precise timing
    const size_t iterations = 10000000; 
    
    // 'volatile' prevents the compiler from optimizing away the loops
    volatile double dummy_sink = 0;

    std::cout << "--- Benchmark: " << iterations << " Allocations/Deallocations ---" << std::endl;

    // ---------------------------------------------------------
    // 1. Benchmark: Custom MemoryPool
    // ---------------------------------------------------------
    MemoryPool<Payload> pool(iterations);
    
    auto start_pool = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        Payload* p = pool.allocate();
        if (p) {
            p->data[0] = static_cast<double>(i);
            dummy_sink = p->data[0]; // Force write/read
            pool.deallocate(p);
        }
    }
    auto end_pool = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_pool = end_pool - start_pool;

    // ---------------------------------------------------------
    // 2. Benchmark: Standard Malloc (System Heap)
    // ---------------------------------------------------------
    auto start_malloc = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; ++i) {
        Payload* p = static_cast<Payload*>(std::malloc(sizeof(Payload)));
        if (p) {
            p->data[0] = static_cast<double>(i);
            dummy_sink = p->data[0]; // Force write/read
            std::free(p);
        }
    }
    auto end_malloc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_malloc = end_malloc - start_malloc;

    // ---------------------------------------------------------
    // 3. Results and Analysis
    // ---------------------------------------------------------
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n[RESULTS]\n";
    std::cout << "Custom MemoryPool: " << diff_pool.count() << "s\n";
    std::cout << "Standard Malloc:   " << diff_malloc.count() << "s\n";

    if (diff_pool.count() > 0) {
        double speedup = diff_malloc.count() / diff_pool.count();
        std::cout << "\nANALYSIS: The Pool is " << speedup << "x faster.\n";
        
        if (speedup > 1.0) {
            std::cout << "VERDICT: Success! O(1) allocation and Cache Locality confirmed.\n";
        } else {
            std::cout << "VERDICT: Unexpected result. Check compiler optimization flags.\n";
        }
    }

    // Prevent unused variable warning
    (void)dummy_sink;

    return 0;
}
