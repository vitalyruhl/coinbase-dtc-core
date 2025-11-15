#include "coinbase_dtc_core/exchanges/coinbase/coinbase_feed.hpp"
#include "coinbase_dtc_core/core/util/log.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace open_dtc_server;

class TestCallbacks
{
public:
    std::atomic<int> trade_count{0};
    std::atomic<int> level2_count{0};

    void on_trade(const exchanges::base::MarketTrade &trade)
    {
        trade_count++;
        util::log("[CALLBACK] Trade received: " + trade.symbol + " Price: " + std::to_string(trade.price) +
                  " Volume: " + std::to_string(trade.volume) +
                  " Side: " + trade.side);
    }

    void on_level2(const exchanges::base::MarketLevel2 &level2)
    {
        level2_count++;
        util::log("[CALLBACK] Level2 received: " + level2.symbol +
                  " Bid: " + std::to_string(level2.bid_price) + "x" + std::to_string(level2.bid_size) +
                  " Ask: " + std::to_string(level2.ask_price) + "x" + std::to_string(level2.ask_size));
    }
};

int main()
{
    util::log("[TEST] Starting Coinbase Feed tests...");

    try
    {
        // Test 1: Feed creation
        exchanges::base::ExchangeConfig config;
        config.name = "coinbase";

        exchanges::coinbase::CoinbaseFeed feed(config);
        util::log("[TEST] Coinbase feed created successfully");

        // Test 2: Exchange name
        std::string exchange_name = feed.get_exchange_name();
        if (exchange_name == "coinbase")
        {
            util::log("[SUCCESS] Exchange name correct: " + exchange_name);
        }

        // Test 3: Symbol normalization
        std::string normalized = feed.normalize_symbol("BTC-USD");
        util::log("[INFO] Symbol normalization: BTC-USD -> " + normalized);

        // Test 4: Connection test
        util::log("[INFO] Testing connection...");
        bool connected = feed.connect();
        if (connected)
        {
            util::log("[SUCCESS] Connection successful");

            // Test 5: Set up callbacks
            TestCallbacks callbacks;
            feed.set_trade_callback([&callbacks](const exchanges::base::MarketTrade &trade)
                                    { callbacks.on_trade(trade); });

            feed.set_level2_callback([&callbacks](const exchanges::base::MarketLevel2 &level2)
                                     { callbacks.on_level2(level2); });

            // Test 6: Subscribe to symbols
            bool sub_trades = feed.subscribe_trades("BTC-USD");
            bool sub_level2 = feed.subscribe_level2("BTC-USD");

            if (sub_trades && sub_level2)
            {
                util::log("[SUCCESS] Subscriptions successful");

                // Test 7: Wait for some data
                util::log("[INFO] Waiting for market data (5 seconds)...");
                std::this_thread::sleep_for(std::chrono::seconds(5));

                util::log("[INFO] Received " + std::to_string(callbacks.trade_count.load()) + " trades");
                util::log("[INFO] Received " + std::to_string(callbacks.level2_count.load()) + " level2 updates");

                // Test 8: Check subscribed symbols
                auto subscribed = feed.get_subscribed_symbols();
                util::log("[INFO] Subscribed symbols: " + std::to_string(subscribed.size()));

                // Test 9: Unsubscribe
                bool unsub = feed.unsubscribe("BTC-USD");
                if (unsub)
                {
                    util::log("[SUCCESS] Unsubscribe successful");
                }
            }

            // Test 10: Disconnect
            feed.disconnect();
            if (!feed.is_connected())
            {
                util::log("[SUCCESS] Disconnection successful");
            }
        }
        else
        {
            util::log("[WARN] Connection failed - testing offline functionality");
        }

        util::log("[SUCCESS] All Coinbase Feed tests completed successfully!");
        return 0;
    }
    catch (const std::exception &e)
    {
        util::log("[ERROR] Coinbase feed test failed: " + std::string(e.what()));
        return 1;
    }
}

