#!/bin/bash
cd ../..
cmake . && make
cp code test/storage/.
cd test/storage
preserve_data=false

for arg in "$@"; do
    if [ "$arg" = "--preserve" ] || [ "$arg" = "-p" ]; then
        preserve_data=true
        shift
    fi
    if [ "$arg" = "--clean" ] || [ "$arg" = "-c" ]; then
        rm -f data_data data_node
        rm -f *.test_out
        shift
    fi
done

if [ $# -eq 0 ]; then
    echo "Usage: $0 [--preserve] <test_number1> [test_number2] ..."
    echo "  --preserve, -p: keeps data files after test"
    echo "  --clean, -c: cleans up data before running tests"
    exit 1
fi

for num in "$@"; do
    infile="${num}.in"
    outfile="${num}.out"
    testout="${num}.test_out"
    if [ ! -f "$infile" ] || [ ! -f "$outfile" ]; then
        echo "Test point $num: input or output file not found!"
        continue
    fi
    ./code < "$infile" > "$testout"
    if ! diff -q "$testout" "$outfile" > /dev/null; then
        echo "Test $num failed:"
        diff "$testout" "$outfile"
    else
        echo "Test $num passed."
    fi
    
    if [ "$preserve_data" = false ]; then
        rm -f data_data data_node
        rm -f *.test_out
    fi
done

rm -f code