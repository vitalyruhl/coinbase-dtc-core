#include "coinbase_dtc_core/core/server/server.hpp"
#include "coinbase_dtc_core/core/util/log.hpp"
#include "coinbase_dtc_core/core/dtc/protocol.hpp"
#include "coinbase_dtc_core/exchanges/base/exchange_feed.hpp"
#include "coinbase_dtc_core/exchanges/factory/exchange_factory.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

// Global server instance for signal handling
coinbase_dtc_core::server::Server* g_server = nullptr;

void signal_handler(int signal) {
    if (g_server) {
        coinbase_dtc_core::util::log("Received shutdown signal, stopping server...");
        g_server->stop();
    }
}

int main() {
    using namespace coinbase_dtc_core;
    
    util::log("[START] CoinbaseDTC Server Starting...");
    
    // Setup signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // Create server configuration
        server::ServerConfig config;
        config.port = 11099;                    // DTC default port
        config.max_clients = 50;                // Allow 50 concurrent clients
        config.enable_logging = true;
        config.heartbeat_interval = 30;
        
        // Create and configure server
        server::Server srv(config);
        g_server = &srv;
        
        // Set up connection handlers
        srv.set_connection_handler([](std::shared_ptr<server::ClientSession> client) {
            util::log("[CONN] New client connected: " + client->get_remote_address());
        });
        
        srv.set_disconnection_handler([](std::shared_ptr<server::ClientSession> client) {
            util::log("[DISC] Client disconnected: " + client->get_username() + " (" + client->get_remote_address() + ")");
        });
        
        // Start the DTC server
        if (!srv.start()) {
            util::log("[ERROR] Failed to start DTC server");
            return 1;
        }
        
        util::log("[OK] DTC Server started successfully!");
        std::cout << srv.status() << std::endl;
        
        // Initialize Coinbase WebSocket feed
        util::log("[COINBASE] Starting Coinbase WebSocket integration...");
        
        auto ws_client = std::make_unique<feed::coinbase::WebSocketClient>();
        
        // Set up trade data callback
        ws_client->set_trade_callback([&srv](const feed::coinbase::TradeData& trade) {
            uint32_t symbol_id = 1; // BTC-USD symbol ID
            srv.broadcast_trade_update(symbol_id, trade.price, trade.size, trade.timestamp);
            util::log("[COINBASE-TRADE] " + trade.product_id + " $" + 
                     std::to_string(trade.price) + " vol:" + std::to_string(trade.size) + 
                     " side:" + trade.side);
        });
        
        // Set up order book callback
        ws_client->set_level2_callback([&srv](const feed::coinbase::Level2Data& level2) {
            uint32_t symbol_id = 1; // BTC-USD symbol ID
            srv.broadcast_bid_ask_update(symbol_id, level2.bid_price, level2.bid_size, 
                                       level2.ask_price, level2.ask_size, level2.timestamp);
            util::log("[COINBASE-BOOK] " + level2.product_id + " Bid:$" + 
                     std::to_string(level2.bid_price) + " x " + std::to_string(level2.bid_size) +
                     " | Ask:$" + std::to_string(level2.ask_price) + " x " + std::to_string(level2.ask_size));
        });
        
        // Connect to Coinbase WebSocket feed
        if (ws_client->connect()) {
            util::log("[OK] Connected to Coinbase WebSocket feed");
            
            // Subscribe to market data
            if (ws_client->subscribe_trades("BTC-USD") && ws_client->subscribe_level2("BTC-USD")) {
                util::log("[OK] Subscribed to BTC-USD trades and level2 data");
            }
        } else {
            util::log("[ERROR] Failed to connect to Coinbase WebSocket feed");
            return 1;
        }
        
        // Main server loop - now driven by WebSocket callbacks
        util::log("[SERVER] Market data now driven by Coinbase WebSocket callbacks");
        while (srv.is_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Optional: periodic status updates
            static uint64_t last_status_time = 0;
            uint64_t current_time = server::Server::get_current_timestamp();
            if (current_time - last_status_time > 60000000) { // Every 60 seconds
                util::log("[STATUS] " + ws_client->get_status());
                last_status_time = current_time;
            }
        }
        
        // Cleanup
        ws_client->disconnect();
        util::log("[COINBASE] Disconnected from Coinbase WebSocket feed");
        
        util::log("[STOP] Server main loop ended");
        
    } catch (const std::exception& e) {
        util::log("[ERROR] Server error: " + std::string(e.what()));
        return 1;
    }
    
    util::log("[OK] CoinbaseDTC Server shutdown complete");
    return 0;
}
