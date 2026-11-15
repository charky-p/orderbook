cmake_minimum_required(VERSION 3.24)
project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)

# Turn off sanitizers for benchmarks
option(ORDERBOOK_SANITIZE "Build with AddressSanitizer and UBSan" ON)
if(ORDERBOOK_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Order book library + app
add_library(orderbook
    src/order_book.cpp
)
target_include_directories(orderbook PUBLIC include)

add_executable(orderbook_app
    apps/main.cpp
    apps/workload_gen.cpp
)
target_link_libraries(orderbook_app PRIVATE orderbook)

# Tests (GoogleTest)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(orderbook_tests
    tests/order_book_test.cpp
)
target_link_libraries(orderbook_tests PRIVATE orderbook GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(orderbook_tests)
