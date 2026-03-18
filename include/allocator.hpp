#pragma once
#include <cstddef>
#include <iostream>

/**
 * @brief MemoryPool: A constant-time O(1) memory manager.
 * Implements an intrinsic Free-List to manage contiguous memory blocks efficiently.
 */
template <typename T>
class MemoryPool {
private:
    // Node structure for the linked list of available blocks (Free-List)
    struct Block {
        Block* next;
    };

    // Strict declaration order: members are initialized in the order they appear here.
    T* pool_start;                  // Base pointer to the raw allocated memory region
    Block* next_free_block;         // Head of the Free-List (next available slot)
    const size_t block_unit_size;   // Aligned size per cell (max of sizeof(T) and sizeof(Block*))
    const size_t total_blocks;      // Maximum capacity of the pool

public:
    /**
     * @brief Constructor: Allocates raw memory and initializes the Free-List.
     * @param num_blocks Number of elements of type T the pool will accommodate.
     */
    MemoryPool(size_t num_blocks) 
        : pool_start(nullptr),
          next_free_block(nullptr),
          block_unit_size(sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block)),
          total_blocks(num_blocks) 
    {
        // Allocate raw memory using operator new to avoid calling default constructors
        pool_start = static_cast<T*>(operator new(block_unit_size * total_blocks));
        next_free_block = reinterpret_cast<Block*>(pool_start);

        // Segment the allocated region and link blocks into the Free-List
        Block* current = next_free_block;
        for (size_t i = 0; i < total_blocks - 1; i++) {
            // Byte-level pointer arithmetic (via char*) to calculate the next block offset
            char* next_ptr = reinterpret_cast<char*>(current) + block_unit_size;
            current->next = reinterpret_cast<Block*>(next_ptr);
            current = current->next;
        }
        current->next = nullptr; // Sentinel value for the end of the list
    }

    /**
     * @brief Destructor: Releases the raw memory back to the system.
     */
    ~MemoryPool() {
        operator delete(pool_start);
    }

    /**
     * @brief Allocates a block from the pool in O(1) time.
     * @return Pointer to type T or nullptr if the pool is exhausted.
     */
    T* allocate() {
        if (!next_free_block) return nullptr;

        // Pop the head of the Free-List
        Block* occupied_block = next_free_block;
        next_free_block = next_free_block->next;
        
        // Reinterpret the memory address as the user's data type
        return reinterpret_cast<T*>(occupied_block);
    }

    /**
     * @brief Returns a block back to the Free-List in O(1) time.
     * @param p Pointer to the memory block to be deallocated.
     */
    void deallocate(T* p) {
        if (!p) return;

        // Push the block back onto the top of the Free-List (LIFO)
        Block* new_free_block = reinterpret_cast<Block*>(p);
        new_free_block->next = next_free_block;
        next_free_block = new_free_block;
    }
};
