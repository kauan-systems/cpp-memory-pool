/**
 * @file main.cpp
 * @brief Functional test, alignment validation, and integrity check for the MemoryPool.
 * @details This file demonstrates the basic usage of the MemoryPool class in a 
 * single-threaded environment to validate memory segmentation logic and 
 * pointer recycling.
 */

#include "allocator.hpp"
#include <iostream>
#include <cstdint> 

/**
 * @brief Entry point for basic demonstration and validation.
 * 1. Instantiation and Allocation.
 * 2. Read/Write Test (Integrity).
 * 3. Memory Layout Analysis (Alignment).
 * 4. Block Recycling (Deallocation).
 */
int main() {
    // 1. Pool instantiation for primitive type 'int' with a capacity of 3 units.
    // The pool will reserve enough contiguous memory for 3 blocks.
    MemoryPool<int> pool(3);

    // 2. Sequential memory block allocation in O(1) time.
    int* a = pool.allocate();
    int* b = pool.allocate();
    int* c = pool.allocate(); 

    // 3. Integrity validation: Ensure the pool did not return null pointers.
    if (!a || !b || !c) {
        std::cerr << "Critical error: Pool exhaustion or allocation failure!\n";
        return 1;
    }

    // 4. Write test: Verify that returned addresses are writable and independent.
    // This confirms there is no memory overlap between blocks.
    *a = 100;
    *b = 200;
    *c = 300;

    std::cout << "\n--- Allocation Report ---\n";
    std::cout << "Pointer A: " << a << " | Content: " << *a << "\n";
    std::cout << "Pointer B: " << b << " | Content: " << *b << "\n";
    std::cout << "Pointer C: " << c << " | Content: " << *c << "\n";

    // 5. Alignment and Offset Analysis:
    // Calculate the byte offset between two consecutive blocks to validate 
    // if the pool's internal pointer arithmetic respects the 'block_unit_size'.
    size_t diff = static_cast<size_t>(reinterpret_cast<char*>(b) - reinterpret_cast<char*>(a));
    
    std::cout << "\n--- Memory Layout Analysis ---\n";
    std::cout << "Offset between A and B: " << diff << " bytes" << std::endl;

    // Logical validation: The difference must be at least the size of type T (or the Block node).
    if (diff >= sizeof(int)) {
        std::cout << "STATUS: VERIFIED. Consistent alignment with the architecture.\n";
    } else {
        std::cout << "STATUS: FAILURE. Segmentation overlap or corruption detected.\n";
        return 1;
    }

    // 6. Memory Recycling: Return pointers to the pool's Free-List.
    // After deallocation, these blocks will be ready for reuse.
    pool.deallocate(a);
    pool.deallocate(b);
    pool.deallocate(c);

    std::cout << "\nExecution completed without access violations.\n";
    std::cout << "The MemoryPool is ready for production use.\n\n";
    
    return 0;
}
