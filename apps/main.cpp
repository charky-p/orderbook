#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>

#include "apps/workload_gen.hpp"
#include "orderbook/order_book.hpp"

using namespace orderbook;

struct BenchmarkStats {
    size_t executedOps;
    double totalMs;
    double throughputOpsPerSec;
    double meanNs;
    int64_t p50Ns; // Median
    int64_t p99Ns; // 99 percentile
    int64_t p999Ns; // 99.9 percentile
    uint64_t sink; // for compiler optimisation
};

BenchmarkStats runBenchmark(const std::vector<OrderEvent>& events) {
    OrderBook book;
    std::vector<OrderId> activeIds;
    activeIds.reserve(events.size());

    std::vector<int64_t> latenciesNs;
    latenciesNs.reserve(events.size());

    uint64_t sink = 0; // Use the trades so compiler does not remove this

    const auto batchStart = std::chrono::steady_clock::now();

    for (const auto& ev : events) {
        if (ev.type == EventType::Add) {
            const auto t0 = std::chrono::steady_clock::now();
            const auto res = book.add(ev.side, ev.price, ev.qty);
            const auto t1 = std::chrono::steady_clock::now();

            latenciesNs.push_back(
                std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()
            );

            sink += res.trades.size();

            Quantity filledQty = 0;
            for (const auto& tr : res.trades) {
                filledQty += tr.qty;
            }
            if (filledQty < ev.qty) {
                activeIds.push_back(res.id);
            }
        } else if (!activeIds.empty()) {
            const size_t idx = static_cast<size_t>(ev.qty) % activeIds.size();
            const OrderId targetId = activeIds[idx];

            // O(1) swap-and-pop removal from live ID tracker
            activeIds[idx] = activeIds.back();
            activeIds.pop_back();

            const auto t0 = std::chrono::steady_clock::now();
            if constexpr (std::is_void_v<decltype(book.cancel(targetId))>) {
                book.cancel(targetId);
                const auto t1 = std::chrono::steady_clock::now();
                latenciesNs.push_back(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()
                );
                sink += 1;
            } else {
                const bool canceled = book.cancel(targetId);
                const auto t1 = std::chrono::steady_clock::now();
                latenciesNs.push_back(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count()
                );
                sink += canceled ? 1 : 0;
            }
        }
    }

    const auto batchEnd = std::chrono::steady_clock::now();
    const double totalNs = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(batchEnd - batchStart).count()
    );

    std::sort(latenciesNs.begin(), latenciesNs.end());
    const size_t n = latenciesNs.size();

    return BenchmarkStats{
        .executedOps = n,
        .totalMs = totalNs / 1e6,
        .throughputOpsPerSec = (static_cast<double>(n) * 1e9) / totalNs,
        .meanNs = totalNs / static_cast<double>(n),
        .p50Ns = latenciesNs[n / 2],
        .p99Ns = latenciesNs[static_cast<size_t>(n * 0.99)],
        .p999Ns = latenciesNs[static_cast<size_t>(n * 0.999)],
        .sink = sink
    };
}

int main() {
    std::cout << "Generating " << numEvents << " GBM order events...\n";
    const auto events = generateGeometricBrownianMotionOrders();

    // Warmup run (populates instruction cache and branch predictors)
    const auto warmup = runBenchmark(events);
    if (warmup.sink == 0xFFFFFFFFFFFFFFFFULL) {
        std::cout << warmup.sink;
    }

    // Measured run
    const auto stats = runBenchmark(events);

    std::cout << "Executed ops: " << stats.executedOps << "\n"
              << "Total time:   " << stats.totalMs << " ms\n"
              << "Throughput:   " << static_cast<uint64_t>(stats.throughputOpsPerSec) << " ops/sec\n"
              << "Mean latency: " << stats.meanNs << " ns\n"
              << "p50 latency:  " << stats.p50Ns << " ns\n"
              << "p99 latency:  " << stats.p99Ns << " ns\n"
              << "p99.9 lat:    " << stats.p999Ns << " ns\n"
              << "Checksum:     " << stats.sink << "\n";

    return 0;
}