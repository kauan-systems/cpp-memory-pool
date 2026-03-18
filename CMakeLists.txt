cmake_minimum_required(VERSION 3.10)
project(MemoryPoolCpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-O3 -Wall -Wextra -Wpedantic)
endif()

include_directories(include)

add_executable(memory_tester src/main.cpp)
add_executable(stress_test src/stress_test.cpp)

set_target_properties(memory_tester PROPERTIES OUTPUT_NAME "unit_test")
set_target_properties(stress_test PROPERTIES OUTPUT_NAME "benchmark")
