#pragma once

#include <vector>
#include <cstdint>
#include <optional>

namespace orderbook {

using Price = std::int64_t;
using Quantity = std::int32_t;
using OrderId = std::uint32_t;
enum class Side { Buy, Sell };

struct Order {
    OrderId id;
    Side side;
    Price price;
    Quantity qty;
};

struct Trade {
    Price price;
    Quantity qty;
    OrderId buyId;
    OrderId sellId;
};

struct AddResult {
    OrderId id;
    std::vector<Trade> trades;
};

class OrderBook {
public:
    // Id is returned, so Id is handled by the order book
    AddResult add(Side side, Price price, Quantity qty);
    Quantity quantityAt(Side side, Price price) const;
    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;
    bool cancel(OrderId id);

    // TODO: Market orders vs limit orders
private:
    OrderId m_nextId = 1;

    // Temporary - naive way for benchmarking, need to use concepts later
    std::vector<Order> m_bids; // descending
    std::vector<Order> m_asks; // ascending
};

}  // namespace orderbook
