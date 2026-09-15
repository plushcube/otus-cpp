#!/bin/bash

echo "Running self-check..."

echo "  Building..."
cmake -S . -B .build -Wno-dev >/dev/null
cmake --build .build >/dev/null

echo "  Testing..."
if ! ctest --test-dir .build --output-on-failure >/dev/null; then
    echo "❌ Tests failed."
    exit 1
fi

echo "✅ Self-check completed successfully."
