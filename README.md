# orderbook

A limit order book and matching engine in C++20.

## Build and test (WSL / Linux)

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/orderbook_app
```

Debug builds run with AddressSanitizer and UBSan, so dangling references and leaks fail loudly.
For benchmarks, build without them:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DORDERBOOK_SANITIZE=OFF
cmake --build build-release -j
```
