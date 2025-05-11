#!/bin/bash

# Configuration parameters
NUM_TESTS=1000  # Number of test cases to generate
DATA_DIR="test_inputs"  # Directory for test data
SAMPLE_PROG="./sample"  # Path to the brute-force program executable
MAIN_PROG="./main"  # Path to your program executable

# 1. Generate test data
g++ main.cpp -o  main -fsanitize=address,leak,undefined
g++ sample.cpp -o  sample -fsanitize=address,leak,undefined

echo "Generating test data..."
mkdir -p $DATA_DIR
for ((i=1; i<=NUM_TESTS; i++)); do
    python3 fxxkdata.py 
    $SAMPLE_PROG < "$DATA_DIR/1.in" > "$DATA_DIR/1.out"
    $MAIN_PROG < "$DATA_DIR/1.in" > "$DATA_DIR/1.myout"
    echo "Comparing outputs..."
    if diff -b "$DATA_DIR/1.out" "$DATA_DIR/1.myout" > /dev/null; then
        echo "Test $i: Passed"
    else
        echo "Test $i: Failed"
        echo "Diff for test $i:"
        diff "$DATA_DIR/$i.out" "$DATA_DIR/$i.myout"
        exit
    fi
done