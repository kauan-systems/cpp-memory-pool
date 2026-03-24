cmake_minimum_required(VERSION 3.10)
project(MemoryPoolCpp VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Forçar flags de hardware específicas para 128-bit CAS
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mcx16 -pthread -Wall")
endif()

include_directories(include)

add_executable(unit_test src/main.cpp)
add_executable(benchmark src/stress_test.cpp)

# A parte crítica: Linkar explicitamente a libatomic
if(UNIX)
    target_link_libraries(unit_test PRIVATE atomic pthread)
    target_link_libraries(benchmark PRIVATE atomic pthread)
endif()
