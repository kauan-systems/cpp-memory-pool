/**
 * @file stress_test.cpp
 * @brief Multithreaded Benchmark: Lock-Free MemoryPool vs. Standard Malloc.
 * @details Evaluates performance using batch allocation to break 
 * immediate thread-local caches and simulate real-world memory pressure.
 * @author Your Name / Your GitHub Username
 * @date 2026
 */

#include "allocator.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstdlib>

/**
 * @brief Prevents the compiler from optimizing away memory operations.
 * @param p Pointer that should "escape" the optimizer's reach.
 */
inline void escape(void* p) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "g"(p) : "memory");
#else
    static volatile void* sink;
    sink = p;
#endif
}

/**
 * @struct Payload
 * @brief Data structure for memory pressure testing (32 bytes).
 */
struct Payload {
    double data[4]; 
};

/**
 * @brief Worker for MemoryPool using the Batch technique.
 * Allocates 1000 objects before deallocating to bypass immediate cache recycling.
 * @param pool Reference to the shared MemoryPool.
 * @param iterations Total number of operations per thread.
 */
void worker_pool(MemoryPool<Payload>& pool, size_t iterations) {
    std::vector<Payload*> batch;
    batch.reserve(1000);

    for (size_t i = 0; i < iterations / 1000; ++i) {
        // Allocate a batch of 1000 blocks
        for (int j = 0; j < 1000; ++j) {
            Payload* p = pool.allocate();
            if (p) {
                p->data[0] = static_cast<double>(i + j);
                escape(p);
                batch.push_back(p);
            }
        }
        // Deallocate the batch of 1000 blocks
        for (auto p : batch) {
            pool.deallocate(p);
        }
        batch.clear();
    }
}

/**
 * @brief Worker for Malloc using the Batch technique.
 * Forces the system allocator to manage multiple simultaneous addresses.
 * @param iterations Total number of operations per thread.
 */
void worker_malloc(size_t iterations) {
    std::vector<Payload*> batch;
    batch.reserve(1000);

    for (size_t i = 0; i < iterations / 1000; ++i) {
        // Allocate a batch of 1000 blocks via Standard Heap
        for (int j = 0; j < 1000; ++j) {
            Payload* p = static_cast<Payload*>(std::malloc(sizeof(Payload)));
            if (p) {
                p->data[0] = static_cast<double>(i + j);
                escape(p);
                batch.push_back(p);
            }
        }
        // Deallocate the batch of 1000 blocks
        for (auto p : batch) {
            std::free(p);
        }
        batch.clear();
    }
}

/**
 * @brief Main benchmark execution engine.
 */
int main() {
    const unsigned int num_threads = std::thread::hardware_concurrency();
    const size_t iterations_per_thread = 1000000; 
    const size_t total_ops = num_threads * iterations_per_thread;

    std::cout << "====================================================\n";
    std::cout << "   BENCHMARK: LOCK-FREE POOL VS STANDARD MALLOC     \n";
    std::cout << "====================================================\n";
    std::cout << "Hardware Cores:   " << num_threads << "\n";
    std::cout << "Ops per Thread:   " << iterations_per_thread << "\n";
    std::cout << "Batch Size:       1000\n";
    std::cout << "====================================================\n" << std::endl;

    // Initialize the Pool with enough capacity to sustain simultaneous batches.
    // num_threads * 1000 is the theoretical minimum; we use a multiplier for safety.
    MemoryPool<Payload> pool(num_threads * 2048); 

    // --- PHASE 1: Custom Lock-Free MemoryPool ---
    std::cout << "Running Custom MemoryPool test..." << std::endl;
    auto start_pool = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::jthread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_pool, std::ref(pool), iterations_per_thread);
        }
    } 
    auto end_pool = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_pool = end_pool - start_pool;

    // --- PHASE 2: Standard Malloc (System Heap) ---
    std::cout << "Running Standard Malloc test..." << std::endl;
    auto start_malloc = std::chrono::high_resolution_clock::now();
    {
        std::vector<std::jthread> threads;
        for (unsigned int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_malloc, iterations_per_thread);
        }
    }
    auto end_malloc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_malloc = end_malloc - start_malloc;

    // --- Final Results ---
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n[FINAL STATISTICS]\n";
    std::cout << "MemoryPool Total Time: " << diff_pool.count() << "s\n";
    std::cout << "Std Malloc Total Time: " << diff_malloc.count() << "s\n";

    if (diff_pool.count() > 0) {
        double throughput = (total_ops / diff_pool.count()) / 1e6;
        double speedup = diff_malloc.count() / diff_pool.count();

        std::cout << "\n[ANALYSIS]\n";
        std::cout << "Throughput: " << throughput << " Million ops/sec\n";
        std::cout << "Performance Ratio: " << speedup << "x (Pool vs Malloc)\n";
        
        if (speedup > 1.0) {
            std::cout << "VERDICT: Lock-Free Pool outperformed Standard Malloc.\n";
        } else {
            std::cout << "VERDICT: Standard Malloc remains competitive in this environment.\n";
        }
    }

    std::cout << "\nBenchmark finished successfully.\n";
    return 0;
}
