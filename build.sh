#!/bin/bash

set -e

if [ "$1" == "clean" ]; then
  printf "Cleaning up build directory...\n"
  rm -rf build
fi

printf "\nStarting Memory Pool build...\n"

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

printf "\nCompiling with CMake...\n\n"
cmake ..
make

cd ..

printf "\n[OK] Build finished successfully!\n\n"
printf "%s\n" "--------------------------------------------------------"
printf "1. Logic & Alignment:    ./build/unit_test\n"
printf "2. Performance Benchmark: ./build/benchmark\n"
printf "%s\n" "--------------------------------------------------------"

printf "\n[STEP 1] Running Functional Validation...\n"
./build/unit_test

printf "[STEP 2] Running Performance Stress Test...\n\n"
./build/benchmark

printf "\nAll tests completed successfully!\n\n"
