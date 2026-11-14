#include <gtest/gtest.h>

#include "orderbook/order_book.hpp"

TEST(OrderBook, EmptyBook) {
    // Check best bid and ask
    // Expect: No best bid or ask
}

TEST(OrderBook, NoTrades) {
    // Buy 5 @ 99
    // Sell 3 @ 100
    // Expect: No trades, resting orders
}

TEST(OrderBook, SellTrade) {
    // Buy 5 @ 99
    // Sell 1 @ 100
    // Sell 2 @ 99
    // Expect: Trade 2 @ 99
}

TEST(OrderBook, BuyTrade) {
    // Sell 3 @ 100
    // Buy 1 @ 99
    // Buy 2 @ 100
    // Expect: Trade 2 @ 100
}

TEST(OrderBook, SellOverTrade) {
    // Buy 5 @ 99
    // Sell 3 @ 200
    // Sell 5 @ 95
    // Expect: Sell the 5 at 99 not 95
}

TEST(OrderBook, BuyOverTrade) {
    // Sell 5 @ 100
    // Buy 3 @ 50
    // Buy 5 @ 105
    // Expect: Buy the 5 at 100 not 105
}

TEST(OrderBook, BuyLeftovers) {
    // Sell 3 @ 99
    // Sell 2 @ 98
    // Buy 8 @ 99
    // Expect: Bid 3 @ 99 at rest in book
}

TEST(OrderBook, SellLeftovers) {
    // Buy 3 @ 100
    // Buy 2 @ 99
    // Sell 8 @ 99
    // Expect: Ask 3 @ 99 at rest in book
}