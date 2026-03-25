cmake_minimum_required(VERSION 3.15)
project(MemoryPoolCpp VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    add_compile_options(
        -O3 
        -march=native 
        -mcx16 
        -pthread 
        -Wall
        -ftls-model=initial-exec
        -fomit-frame-pointer
    )
endif()

include_directories(include .)

enable_testing()

add_executable(unit_test src/main.cpp)
add_executable(benchmark src/stress_test.cpp)

if(UNIX)
    target_link_libraries(unit_test PRIVATE atomic pthread)
    target_link_libraries(benchmark PRIVATE atomic pthread)
endif()

add_test(NAME RunUnitVerify COMMAND unit_test)
add_test(NAME RunBenchmark COMMAND benchmark)

set_tests_properties(RunBenchmark PROPERTIES DEPENDS RunUnitVerify)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
