#pragma once

#include "orderbook/order_book.hpp"

namespace orderbook {

enum class EventType : uint8_t {
    Add,
    Cancel
};

struct OrderEvent {
    EventType type;
    Side side;
    Price price;
    Quantity qty;
};

constexpr size_t numEvents = 100'000;
constexpr uint64_t seed = 100;
constexpr Price initialPrice = 10'000; // $100.00
constexpr double drift = 0.0;
constexpr double volatility = 0.05;
constexpr double cancelRatio = 0.30; // cancel % of orders

std::vector<OrderEvent> generateGeometricBrownianMotionOrders();

} // namespace orderbook