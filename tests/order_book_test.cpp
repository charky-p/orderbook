#include <gtest/gtest.h>

#include "orderbook/order_book.hpp"

using namespace orderbook;

// Basic order book tests

TEST(OrderBook, EmptyBook) {
    // Check best bid and ask
    // Expect: No best bid or ask

    OrderBook book;
    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_EQ(book.bestBid(), std::nullopt);
}

TEST(OrderBook, NoTrades) {
    // Buy 5 @ 99
    // Sell 3 @ 100
    // Expect: No trades, resting orders

    OrderBook book;
    const auto res1 = book.add(Side::Buy, 99, 5);
    const auto res2 = book.add(Side::Sell, 100, 3);

    EXPECT_TRUE(res1.trades.empty());
    EXPECT_TRUE(res2.trades.empty());
}

TEST(OrderBook, SellTrade) {
    // Buy 5 @ 99
    // Sell 1 @ 100
    // Sell 2 @ 99
    // Expect: Trade 2 @ 99

    OrderBook book;
    const auto res1 = book.add(Side::Buy, 99, 5);
    const auto res2 = book.add(Side::Sell, 100, 1);
    const auto res3 = book.add(Side::Sell, 99, 2);

    EXPECT_TRUE(res1.trades.empty());
    EXPECT_TRUE(res2.trades.empty());
    ASSERT_EQ(res3.trades.size(), 1);

    const auto trade = res3.trades[0];
    EXPECT_EQ(trade.buyId, res1.id);
    EXPECT_EQ(trade.sellId, res3.id);
    EXPECT_EQ(trade.price, 99);
    EXPECT_EQ(trade.qty, 2);
}

TEST(OrderBook, BuyTrade) {
    // Sell 3 @ 100
    // Buy 1 @ 99
    // Buy 2 @ 100
    // Expect: Trade 2 @ 100

    OrderBook book;
    const auto res1 = book.add(Side::Sell, 100, 3);
    const auto res2 = book.add(Side::Buy, 99, 1);
    const auto res3 = book.add(Side::Buy, 100, 2);

    EXPECT_TRUE(res1.trades.empty());
    EXPECT_TRUE(res2.trades.empty());
    ASSERT_EQ(res3.trades.size(), 1);

    const auto trade = res3.trades[0];
    EXPECT_EQ(trade.buyId, res3.id);
    EXPECT_EQ(trade.sellId, res1.id);
    EXPECT_EQ(trade.price, 100);
    EXPECT_EQ(trade.qty, 2);
}

TEST(OrderBook, SellOverTrade) {
    // Buy 5 @ 99
    // Sell 3 @ 200
    // Sell 5 @ 95
    // Expect: Sell the 5 at 99 not 95

    
    OrderBook book;
    const auto res1 = book.add(Side::Buy, 99, 5);
    const auto res2 = book.add(Side::Sell, 200, 3);
    const auto res3 = book.add(Side::Sell, 95, 5);

    ASSERT_EQ(res3.trades.size(), 1);

    const auto trade = res3.trades[0];
    EXPECT_EQ(trade.buyId, res1.id);
    EXPECT_EQ(trade.sellId, res3.id);
    EXPECT_EQ(trade.price, 99);
    EXPECT_EQ(trade.qty, 5);
}

TEST(OrderBook, BuyOverTrade) {
    // Sell 5 @ 100
    // Buy 3 @ 50
    // Buy 5 @ 105
    // Expect: Buy the 5 at 100 not 105

    OrderBook book;
    const auto res1 = book.add(Side::Sell, 100, 5);
    book.add(Side::Buy, 50, 3);
    const auto res3 = book.add(Side::Buy, 105, 5);

    ASSERT_EQ(res3.trades.size(), 1);

    const auto trade = res3.trades[0];
    EXPECT_EQ(trade.buyId, res3.id);
    EXPECT_EQ(trade.sellId, res1.id);
    EXPECT_EQ(trade.price, 100);
    EXPECT_EQ(trade.qty, 5);

    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_NE(book.bestBid(), std::nullopt);
}

TEST(OrderBook, BuyLeftovers) {
    // Sell 3 @ 99
    // Sell 2 @ 98
    // Buy 8 @ 99
    // Expect: Trades (2 @ 98, then 3 @ 99), leaving Bid 3 @ 99 at rest in book

    OrderBook book;
    const auto res1 = book.add(Side::Sell, 99, 3);
    const auto res2 = book.add(Side::Sell, 98, 2);
    const auto res3 = book.add(Side::Buy, 99, 8);

    ASSERT_EQ(res3.trades.size(), 2);

    // Best ask (98) must execute before 99
    EXPECT_EQ(res3.trades[0].buyId, res3.id);
    EXPECT_EQ(res3.trades[0].sellId, res2.id);
    EXPECT_EQ(res3.trades[0].price, 98);
    EXPECT_EQ(res3.trades[0].qty, 2);

    EXPECT_EQ(res3.trades[1].buyId, res3.id);
    EXPECT_EQ(res3.trades[1].sellId, res1.id);
    EXPECT_EQ(res3.trades[1].price, 99);
    EXPECT_EQ(res3.trades[1].qty, 3);

    EXPECT_EQ(book.bestAsk(), std::nullopt);
    ASSERT_NE(book.bestBid(), std::nullopt);

    // Probe with Sell 4 @ 99 to verify exactly 3 units of res3 remain resting at 99
    const auto probe = book.add(Side::Sell, 99, 4);
    ASSERT_EQ(probe.trades.size(), 1);
    EXPECT_EQ(probe.trades[0].buyId, res3.id);
    EXPECT_EQ(probe.trades[0].sellId, probe.id);
    EXPECT_EQ(probe.trades[0].price, 99);
    EXPECT_EQ(probe.trades[0].qty, 3);
    EXPECT_EQ(book.bestBid(), std::nullopt);
}

TEST(OrderBook, SellLeftovers) {
    // Buy 3 @ 100
    // Buy 2 @ 99
    // Sell 8 @ 99
    // Expect: Trades (3 @ 100, then 2 @ 99), leaving Ask 3 @ 99 at rest in book

    OrderBook book;
    const auto res1 = book.add(Side::Buy, 100, 3);
    const auto res2 = book.add(Side::Buy, 99, 2);
    const auto res3 = book.add(Side::Sell, 99, 8);

    ASSERT_EQ(res3.trades.size(), 2);

    // Best bid (100) must execute before 99
    EXPECT_EQ(res3.trades[0].buyId, res1.id);
    EXPECT_EQ(res3.trades[0].sellId, res3.id);
    EXPECT_EQ(res3.trades[0].price, 100);
    EXPECT_EQ(res3.trades[0].qty, 3);

    EXPECT_EQ(res3.trades[1].buyId, res2.id);
    EXPECT_EQ(res3.trades[1].sellId, res3.id);
    EXPECT_EQ(res3.trades[1].price, 99);
    EXPECT_EQ(res3.trades[1].qty, 2);

    EXPECT_EQ(book.bestBid(), std::nullopt);
    ASSERT_NE(book.bestAsk(), std::nullopt);

    // Probe with Buy 4 @ 99 to verify exactly 3 units of res3 remain resting at 99
    const auto probe = book.add(Side::Buy, 99, 4);
    ASSERT_EQ(probe.trades.size(), 1);
    EXPECT_EQ(probe.trades[0].buyId, probe.id);
    EXPECT_EQ(probe.trades[0].sellId, res3.id);
    EXPECT_EQ(probe.trades[0].price, 99);
    EXPECT_EQ(probe.trades[0].qty, 3);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}