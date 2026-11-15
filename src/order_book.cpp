#include "orderbook/order_book.hpp"

#include <algorithm>

namespace orderbook {

namespace {
    bool pricesCross(Side incomingSide, Price incomingPrice, Price restingPrice) {
        return (incomingSide == Side::Buy) ? (incomingPrice >= restingPrice) : (incomingPrice <= restingPrice);
    }

    void insertRestingOrder(std::vector<Order>& ownSide, const Order& order) {
        if (order.side == Side::Buy) {
            auto it = std::upper_bound(ownSide.begin(), ownSide.end(), order.price,
                [](Price newPrice, const Order& existing) {
                    return newPrice > existing.price;
                });
            ownSide.insert(it, order);
        } else {
            auto it = std::upper_bound(ownSide.begin(), ownSide.end(), order.price,
                [](Price newPrice, const Order& existing) {
                    return newPrice < existing.price;
                });
            ownSide.insert(it, order);
        }
    }
} // namespace for helpers

    AddResult OrderBook::add(Side side, Price price, Quantity qty) {
        const OrderId id = m_nextId++;
        std::vector<Order>& oppositeSide = (side == Side::Buy) ? m_asks : m_bids;
        std::vector<Order>& ownSide = (side == Side::Buy) ? m_bids : m_asks;

        Quantity remaining = qty;
        std::vector<Trade> trades;

        // Execute trades
        while (remaining > 0 && !oppositeSide.empty() && pricesCross(side, price, oppositeSide.front().price)) {
            // Trade oppositeSide.front()
            Order& tradeOrder = oppositeSide.front();
            Quantity tradeQty = std::min(tradeOrder.qty, remaining);
            OrderId buyId = (side == Side::Buy) ? id : tradeOrder.id;
            OrderId sellId = (side == Side::Buy) ? tradeOrder.id : id;
            Trade trade = {tradeOrder.price, tradeQty, buyId, sellId};
            trades.push_back(trade);
            
            // Update orders
            remaining -= tradeQty;
            tradeOrder.qty -= tradeQty;
            if (tradeOrder.qty <= 0) {
                oppositeSide.erase(oppositeSide.begin());
            }
        }

        // Resting order
        if (remaining > 0) {
            insertRestingOrder(ownSide, Order{id, side, price, remaining});
        }

        return {id, std::move(trades)};
    }

    Quantity OrderBook::quantityAt(Side side, Price price) const {
        const std::vector<Order>& bookSide = (side == Side::Buy) ? m_bids : m_asks;

        // Naive O(N) slow way
        Quantity total = 0;
        for (const Order& order : bookSide) {
            if (order.price == price) {
                total += order.qty;
            }
        }
        return total;
    }

    std::optional<Price> OrderBook::bestBid() const {
        if (m_bids.empty()) {
            return std::nullopt;
        }

        return m_bids.front().price;
    }

    std::optional<Price> OrderBook::bestAsk() const {
        if (m_asks.empty()) {
            return std::nullopt;
        }

        return m_asks.front().price;
    }

    bool OrderBook::cancel(OrderId id) {
        // TODO
        return true;
    }
}  // namespace orderbook
