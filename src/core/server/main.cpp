#include "coinbase_dtc_core/core/server/server.hpp"
#include "coinbase_dtc_core/core/util/log.hpp"
#include "coinbase_dtc_core/exchanges/base/exchange_feed.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

// Global server instance for signal handling
coinbase_dtc_core::core::server::DTCServer *g_server = nullptr;

void signal_handler(int signal)
{
    if (g_server)
    {
        open_dtc_server::util::log("Received shutdown signal, stopping server...");
        g_server->stop();
    }
}

int main()
{
    using namespace coinbase_dtc_core::core::server;

    open_dtc_server::util::log("[START] CoinbaseDTC Server Starting...");

    // Setup signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try
    {
        // Create server configuration
        ServerConfig config;
        config.bind_address = "0.0.0.0"; // Allow connections from any interface
        config.port = 11099;
        config.server_name = "CoinbaseDTCServer";
        config.password = "";
        config.require_authentication = false;

        // Create server instance
        DTCServer srv(config);
        g_server = &srv;

        // Add Coinbase exchange for real market data
        open_dtc_server::exchanges::base::ExchangeConfig coinbase_config;
        coinbase_config.name = "coinbase"; // Must match factory name
        coinbase_config.websocket_url = "wss://ws-feed.exchange.coinbase.com";
        coinbase_config.api_url = "https://api.exchange.coinbase.com";
        coinbase_config.port = 443;
        coinbase_config.requires_auth = false; // For public market data
        // Note: symbols will be configured separately through subscribe calls

        if (!srv.add_exchange(coinbase_config))
        {
            open_dtc_server::util::log("Warning: Failed to add Coinbase exchange - continuing with mock data");
        }
        else
        {
            open_dtc_server::util::log("✅ Added Coinbase exchange for real market data");

            // Subscribe to specific symbols
            srv.subscribe_symbol("BTC-USD", "coinbase");
            srv.subscribe_symbol("ETH-USD", "coinbase");
            srv.subscribe_symbol("SOL-USD", "coinbase");
        }
        open_dtc_server::util::log("Server configured, starting...");

        if (!srv.start())
        {
            open_dtc_server::util::log("Failed to start server");
            return 1;
        }

        open_dtc_server::util::log("DTC Server started successfully");
        open_dtc_server::util::log("Server status: " + srv.get_status());

        // Keep server running
        while (srv.is_running())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        open_dtc_server::util::log("CoinbaseDTC Server shutdown complete");
        return 0;
    }
    catch (const std::exception &e)
    {
        open_dtc_server::util::log("Server error: " + std::string(e.what()));
        return 1;
    }
}