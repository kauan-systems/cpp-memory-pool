#pragma once
#include <cstddef>
#include <atomic>
#include <cstdint>

/**
 * @brief Macro to force inlining across different compilers.
 * Vital for high-performance allocators to eliminate function call overhead in hot paths.
 */
#if defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE inline
#endif

/**
 * @brief A high-performance, lock-free, thread-aware Memory Pool.
 * * Uses a two-layer allocation strategy:
 * 1. L1: Thread-Local Storage (TLS) cache for zero-contention fast paths.
 * 2. L2: Global lock-free stack using 128-bit Atomic Compare-and-Swap (CAS).
 * * @tparam T The type of object to be managed by the pool.
 */
template <typename T>
class MemoryPool {
private:
    /** @brief Internal structure representing a free memory block. */
    struct Block { Block* next; };

    /** * @brief ABA-resistant tagged pointer for lock-free operations.
     * alignas(16) is required for double-width CAS operations (DWCAS).
     */
    struct alignas(16) TaggedPointer { 
        Block* ptr; 
        uintptr_t tag; 
    };

    /** @brief L1 Layer: Private thread-local cache to bypass global atomic contention. */
    static constexpr size_t L1_SIZE = 2048;
    
    /** @brief Refill/Flush batch size to amortize the cost of atomic operations. */
    static constexpr size_t BATCH = 256;

    /** @brief Storage structure for the thread-local L1 cache. */
    struct LocalCache {
        Block* storage[L1_SIZE];
        size_t count = 0;
    };
    
    /** @brief Thread-local instance of the L1 cache. */
    inline static thread_local LocalCache l1;

    T* pool_start;
    const size_t block_unit_size;
    
    /** * @brief L2 Layer: Global atomic head. 
     * alignas(64) prevents "False Sharing" by isolating the head on its own cache line.
     */
    alignas(64) std::atomic<TaggedPointer> head;

public:
    /**
     * @brief Constructs the Memory Pool and pre-allocates a contiguous block of memory.
     * @param n Number of blocks to pre-allocate.
     */
    MemoryPool(size_t n) : block_unit_size(sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block)) {
        pool_start = static_cast<T*>(operator new(block_unit_size * n));
        Block* curr = reinterpret_cast<Block*>(pool_start);
        
        // Link all blocks in a linear chain initially
        for (size_t i = 0; i < n - 1; i++) {
            curr->next = reinterpret_cast<Block*>(reinterpret_cast<char*>(curr) + block_unit_size);
            curr = curr->next;
        }
        curr->next = nullptr;
        
        // Initialize global head with the first block
        head.store({reinterpret_cast<Block*>(pool_start), 0}, std::memory_order_release);
    }

    /** @brief Destructor: Releases the entire memory block allocated by the pool. */
    ~MemoryPool() { operator delete(pool_start); }

    // Disable copy/assignment to prevent double-frees or pointer corruption
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * @brief Allocates a block of memory from the pool.
     * @return T* Pointer to the allocated memory, or nullptr if the pool is exhausted.
     */
    [[nodiscard]] FORCE_INLINE T* allocate() {
        // Path 1: If L1 is empty, attempt to refill from L2 Global Root
        if (l1.count == 0) {
            for(size_t i = 0; i < BATCH; ++i) {
                TaggedPointer old = head.load(std::memory_order_acquire);
                if (!old.ptr) break;
                
                // Atomic DWCAS to move the global stack head
                if (head.compare_exchange_weak(old, {old.ptr->next, old.tag + 1}, 
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                    l1.storage[l1.count++] = old.ptr;
                }
            }
        }
        
        // Path 2: Return from L1 cache (High-speed path)
        return (l1.count > 0) ? reinterpret_cast<T*>(l1.storage[--l1.count]) : nullptr;
    }

    /**
     * @brief Deallocates memory and returns it to the pool.
     * @param p Pointer to the memory block to be returned.
     */
    FORCE_INLINE void deallocate(T* p) {
        if (!p) return;

        // Path 1: Store in L1 cache if space is available
        if (l1.count < L1_SIZE) {
            l1.storage[l1.count++] = reinterpret_cast<Block*>(p);
            return;
        }
        
        // Path 2: L1 is full, flush current block to the L2 Global Root
        Block* node = reinterpret_cast<Block*>(p);
        TaggedPointer old = head.load(std::memory_order_relaxed);
        do {
            node->next = old.ptr;
        } while (!head.compare_exchange_weak(old, {node, old.tag + 1}, 
            std::memory_order_acq_rel, std::memory_order_relaxed));
    }
};
