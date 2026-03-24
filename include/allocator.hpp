/**
 * @file allocator.hpp
 * @brief High-performance, Lock-Free Memory Pool with ABA protection.
 * @details This implementation uses a Lock-Free Singly Linked List (Stack) 
 * approach with 128-bit atomic operations (Double-Width CAS) to manage 
 * memory blocks without mutexes.
 */

#pragma once
#include <cstddef>
#include <atomic>
#include <cstdint>

/**
 * @class MemoryPool
 * @brief A thread-safe, fixed-size memory allocator.
 * @tparam T The type of object to be managed by the pool.
 */
template <typename T>
class MemoryPool {
private:
    /**
     * @brief Internal structure representing a free block of memory.
     * Only used when the block is not allocated to the user.
     */
    struct Block {
        Block* next;
    };

    /**
     * @brief 16-byte (128-bit) structure to prevent the ABA problem.
     * Combines a raw pointer with a generational counter (tag).
     */
    struct alignas(16) TaggedPointer {
        Block* ptr;    ///< Pointer to the memory block
        uintptr_t tag; ///< Incremental version counter to detect re-use
    };

    T* pool_start;              ///< Start address of the raw memory chunk
    const size_t block_unit_size; ///< Size of each block (max of T and Block)
    const size_t total_blocks;    ///< Maximum capacity of the pool
    
    /**
     * @brief Atomic head of the free-list.
     * Forced 16-byte alignment to support CMPXCHG16B on x86_64 architectures.
     */
    alignas(16) std::atomic<TaggedPointer> next_free_block;

public:
    /**
     * @brief Constructor: Pre-allocates a contiguous block of memory.
     * @param num_blocks The number of elements the pool can hold.
     */
    MemoryPool(size_t num_blocks) 
        : pool_start(nullptr),
          block_unit_size(sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block)),
          total_blocks(num_blocks) 
    {
        // Allocate raw memory for the entire pool
        pool_start = static_cast<T*>(operator new(block_unit_size * total_blocks));
        
        Block* first_block = reinterpret_cast<Block*>(pool_start);
        Block* current = first_block;

        // Link all blocks together in a free-list
        for (size_t i = 0; i < total_blocks - 1; i++) {
            char* next_ptr = reinterpret_cast<char*>(current) + block_unit_size;
            current->next = reinterpret_cast<Block*>(next_ptr);
            current = current->next;
        }
        current->next = nullptr;

        // Initialize the atomic head pointing to the first block
        next_free_block.store({first_block, 0}, std::memory_order_release);
    }

    /**
     * @brief Destructor: Frees the entire memory region.
     */
    ~MemoryPool() {
        operator delete(pool_start);
    }

    // Disable copy constructor and assignment to prevent memory corruption
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    /**
     * @brief Allocates a block from the pool.
     * @return T* Pointer to the allocated memory, or nullptr if the pool is empty.
     */
    T* allocate() {
        TaggedPointer old_head = next_free_block.load(std::memory_order_acquire);
        while (old_head.ptr) {
            // Prepare the new head (pointing to the next element in the list)
            TaggedPointer new_head = {old_head.ptr->next, old_head.tag + 1};

            // Atomic Compare-and-Swap: ensure the head hasn't changed since we loaded it
            if (next_free_block.compare_exchange_weak(old_head, new_head, 
                std::memory_order_acq_rel, std::memory_order_acquire)) 
            {
                return reinterpret_cast<T*>(old_head.ptr);
            }
            // Loop continues if compare_exchange fails due to contention
        }
        return nullptr;
    }

    /**
     * @brief Returns a block to the pool, making it available for re-allocation.
     * @param p Pointer to the memory block to deallocate.
     */
    void deallocate(T* p) {
        if (!p) return;

        Block* new_node = reinterpret_cast<Block*>(p);
        TaggedPointer old_head = next_free_block.load(std::memory_order_relaxed);
        TaggedPointer new_head;

        do {
            // Re-link the deallocated node to the current head
            new_node->next = old_head.ptr;
            // Increment the tag to prevent ABA issues during simultaneous access
            new_head = {new_node, old_head.tag + 1};
        } while (!next_free_block.compare_exchange_weak(old_head, new_head,
            std::memory_order_acq_rel, std::memory_order_relaxed));
    }
};
