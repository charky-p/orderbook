#include "apps/workload_gen.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>

namespace orderbook {

std::vector<OrderEvent> generateGeometricBrownianMotionOrders() {
    std::vector<OrderEvent> events;
    events.reserve(numEvents);

    // Geometric Brownian Motion: dS = mu S dt + sigma S dW
    // By Ito's formula: S(t + dt) = S(t) exp((mu - sigma^2 / 2) * dt + sigma * sqrt(dt) Z), Z sim N(0, 1)
    const double dt = 1.0 / static_cast<double>(numEvents);
    const double driftStep = (drift - 0.5 * volatility * volatility) * dt;
    const double diffStep = volatility * std::sqrt(dt);

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> normal(0.0, 1.0);
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    std::uniform_int_distribution<int64_t> offset(-5, 15);
    std::uniform_int_distribution<Quantity> qtyDist(1, 50);
    std::uniform_int_distribution<Quantity> indexDist(0, std::numeric_limits<Quantity>::max());

    double spot = static_cast<double>(initialPrice);

    for (size_t i = 0; i < numEvents; ++i) {
        // Exact GBM simulation
        spot *= std::exp(driftStep + diffStep * normal(rng));

        if (prob(rng) < cancelRatio) {
            events.push_back(OrderEvent{
                .type = EventType::Cancel,
                .side = Side::Buy, // Unused for Cancel
                .price = 0,        // Unused for Cancel
                .qty = indexDist(rng)
            });
            continue;
        }

        const Side side = (prob(rng) < 0.5) ? Side::Buy : Side::Sell;
        const int64_t mid = std::llround(spot);
        const int64_t signedPrice = mid + (side == Side::Buy ? -1 : 1) * offset(rng);
        const Price orderPrice = static_cast<Price>(std::max<int64_t>(1, signedPrice));

        events.push_back(OrderEvent{
            .type = EventType::Add,
            .side = side,
            .price = orderPrice,
            .qty = qtyDist(rng)
        });
    }

    return events;
}

} // namespace orderbook