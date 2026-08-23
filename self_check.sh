#!/bin/bash

echo "Running self-check..."

echo "  Building (normal)..."
cmake -S . -B .build -Wno-dev >/dev/null
cmake --build .build >/dev/null

echo "  Testing (normal)..."
if ! ctest --test-dir .build --output-on-failure >/dev/null; then
    echo "❌ Normal tests failed."
    exit 1
fi

echo "  Building (tsan)..."
cmake -S . -B .build-tsan -DASYNC_TSAN=ON -Wno-dev >/dev/null
cmake --build .build-tsan >/dev/null

echo "  Testing (tsan)..."
if ! ctest --test-dir .build-tsan --output-on-failure >/dev/null; then
    echo "❌ TSan tests failed."
    exit 1
fi

echo "  Testing (stability)..."
if ! ctest --test-dir .build -R MtFixture --repeat until-fail:20 --output-on-failure >/dev/null; then
    echo "❌ Stability check failed."
    exit 1
fi

echo "✅ Self-check completed successfully."
