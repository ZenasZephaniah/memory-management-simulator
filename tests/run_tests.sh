#!/bin/bash

# Navigate to project root (one level up)
cd "$(dirname "$0")/.."

echo "---------------------------------------"
echo "RUNNING MEMORY SIMULATION TEST SUITE"
echo "---------------------------------------"

# Ensure executable exists
if [ ! -f ./mms ]; then
    echo "Executable not found! Compiling..."
    make
fi

# Create logs folder if missing
mkdir -p logs

# Run the simulator with input from tests/workload.txt
./mms < tests/workload.txt > logs/test_results.log

echo "Test Complete! Logs saved to logs/test_results.log"
echo "Verifying results..."

# Verify
if grep -q "L1 HIT" logs/test_results.log; then
    echo "[PASS] Cache Hit Logic detected."
else
    echo "[WARN] No Cache Hits found."
fi

if grep -q "Merged" logs/test_results.log; then
    echo "[PASS] Memory Coalescing detected."
else
    echo "[FAIL] Memory Coalescing failed."
fi