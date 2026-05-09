#!/bin/bash

echo "Running self-check..."

echo "  Building..."
cmake -S . -B .build -Wno-dev 2>&1 > /dev/null
cmake --build .build 2>&1 > /dev/null
echo "  Testing..."
ctest --test-dir .build 2>&1 > /dev/null
if [ $? -eq 0 ]; then
    echo "✅ Tests passed."
else
    echo "❌ Tests failed. Self-check aborted."
    exit 1
fi

if [ "$1" == "" ]; then
    echo "❌ No input file provided. Self-check aborted."
    exit 1
fi

echo "  Checking output for $1 ..."
SUM=$(cat "$1" | .build/ip_filter | md5sum)
echo "  MD5 sum:  $SUM"
if [ "$SUM" == "24e7a7b2270daee89c64d3ca5fb3da1a  -" ]; then
    echo "✅ Output matches expected."
else
    echo "❌ Output does not match expected. Self-check failed."
    exit 1
fi

echo "✅ Self-check completed successfully."
