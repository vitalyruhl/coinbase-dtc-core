#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <map>
#include <unordered_set>
#include <cstring>
#include "coinbase_dtc_core/dtc/protocol.hpp"
#include "symbol_manager.hpp"
#include "coinbase_dtc_core/feed/exchange_feed.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

namespace coinbase_dtc_core {
namespace server {

// Forward declarations
class ClientSession;

// DTC Server Configuration
struct ServerConfig {
    uint16_t port = 11099;              // Default DTC port
    uint32_t max_clients = 100;         // Maximum concurrent clients
    uint32_t buffer_size = 8192;        // Socket buffer size
    uint32_t heartbeat_interval = 30;   // Heartbeat interval in seconds
    bool enable_logging = true;         // Enable detailed logging
    
    ServerConfig() = default;
};

// Client connection state
enum class ClientState {
    CONNECTED = 0,
    AUTHENTICATING = 1, 
    AUTHENTICATED = 2,
    SUBSCRIBED = 3,
    DISCONNECTING = 4,
    DISCONNECTED = 5
};

// Client Session Management
class ClientSession {
public:
    ClientSession(int socket, const std::string& remote_addr);
    ~ClientSession();
    
    // Session management
    bool is_connected() const { return state_ != ClientState::DISCONNECTED; }
    bool is_authenticated() const { return state_ >= ClientState::AUTHENTICATED; }
    ClientState get_state() const { return state_; }
    void set_state(ClientState state) { state_ = state; }
    
    // Socket operations
    int get_socket() const { return socket_; }
    std::string get_remote_address() const { return remote_addr_; }
    
    // Message handling
    bool send_message(const dtc::DTCMessage& message);
    bool send_raw_data(const std::vector<uint8_t>& data);
    
    // Session info
    void set_username(const std::string& username) { username_ = username; }
    std::string get_username() const { return username_; }
    uint64_t get_connection_time() const { return connection_time_; }
    
    // Symbol subscription management
    bool subscribe_symbol(uint32_t symbol_id, const std::string& symbol_name);
    bool unsubscribe_symbol(uint32_t symbol_id);
    bool is_subscribed_to_symbol(uint32_t symbol_id) const;
    std::vector<uint32_t> get_subscribed_symbols() const;
    size_t get_subscription_count() const;
    
private:
    int socket_;
    std::string remote_addr_;
    std::string username_;
    ClientState state_;
    uint64_t connection_time_;
    dtc::Protocol protocol_;
    std::mutex send_mutex_;
    
    // Symbol subscription tracking
    mutable std::mutex subscriptions_mutex_;
    std::unordered_set<uint32_t> subscribed_symbols_;
    std::unordered_map<uint32_t, std::string> symbol_names_;
};

// Message Handler Callback Types
using MessageHandler = std::function<void(std::shared_ptr<ClientSession>, std::unique_ptr<dtc::DTCMessage>)>;
using ConnectionHandler = std::function<void(std::shared_ptr<ClientSession>)>;

// Main DTC Server Class
class Server {
public:
    Server(const ServerConfig& config = ServerConfig());
    ~Server();
    
    // Server lifecycle
    bool start();
    void stop();
    bool is_running() const { return running_; }
    std::string status() const;
    
    // Configuration
    const ServerConfig& get_config() const { return config_; }
    void set_config(const ServerConfig& config) { config_ = config; }
    
    // Client management
    std::vector<std::shared_ptr<ClientSession>> get_clients() const;
    std::shared_ptr<ClientSession> get_client_by_username(const std::string& username) const;
    size_t get_client_count() const;
    void disconnect_client(std::shared_ptr<ClientSession> client);
    void disconnect_all_clients();
    
    // Message handling
    void set_message_handler(dtc::MessageType type, MessageHandler handler);
    void set_connection_handler(ConnectionHandler handler);
    void set_disconnection_handler(ConnectionHandler handler);
    
    // Market data broadcasting
    void broadcast_trade_update(uint32_t symbol_id, double price, double volume, uint64_t timestamp);
    void broadcast_bid_ask_update(uint32_t symbol_id, double bid_price, double bid_qty, 
                                  double ask_price, double ask_qty, uint64_t timestamp);
    
    // WebSocket market data callbacks
    void on_trade_data(const feed::MarketTrade& trade);
    void on_level2_data(const feed::MarketLevel2& level2);
    
    // Dynamic symbol management
    bool handle_market_data_request(std::shared_ptr<ClientSession> client, const dtc::MarketDataRequest& request);
    void update_websocket_subscriptions();
    std::vector<std::string> get_active_symbols() const;
    
    // Exchange management
    bool add_exchange(const std::string& exchange_name);
    bool remove_exchange(const std::string& exchange_name);
    std::vector<std::string> get_active_exchanges() const;
    void broadcast_market_data(const dtc::MarketDataUpdateTrade& update);
    void broadcast_market_data(const dtc::MarketDataUpdateBidAsk& update);
    
    // Utility functions
    static std::string get_local_ip();
    static uint64_t get_current_timestamp();

private:
    // Core server functions
    void accept_loop();
    void handle_client(std::shared_ptr<ClientSession> client);
    bool process_client_messages(std::shared_ptr<ClientSession> client);
    
    // WebSocket initialization
    void initialize_websocket_client();
    
    // Message processing
    void handle_logon_request(std::shared_ptr<ClientSession> client, std::unique_ptr<dtc::DTCMessage> message);
    void handle_market_data_request(std::shared_ptr<ClientSession> client, std::unique_ptr<dtc::DTCMessage> message);
    void handle_heartbeat(std::shared_ptr<ClientSession> client);
    
    // Socket utilities
    bool init_socket();
    void cleanup_socket();
    bool set_socket_options(int socket);
    
    // Client management
    void add_client(std::shared_ptr<ClientSession> client);
    void remove_client(std::shared_ptr<ClientSession> client);
    void cleanup_disconnected_clients();
    
private:
    ServerConfig config_;
    std::atomic<bool> running_;
    std::atomic<bool> shutdown_requested_;
    
    // Socket management
    int server_socket_;
    struct sockaddr_in server_addr_;
    
    // Symbol management
    std::unique_ptr<SymbolManager> symbol_manager_;
    
    // Dynamic subscription tracking
    mutable std::mutex active_symbols_mutex_;
    std::unordered_set<std::string> active_websocket_symbols_;
    
    // Threading
    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;
    
    // Client management
    std::vector<std::shared_ptr<ClientSession>> clients_;
    mutable std::mutex clients_mutex_;
    
    // Market data components
    std::unique_ptr<feed::MultiExchangeFeed> multi_exchange_feed_;
    
    // Message handlers
    std::map<dtc::MessageType, MessageHandler> message_handlers_;
    ConnectionHandler connection_handler_;
    ConnectionHandler disconnection_handler_;
    std::mutex handlers_mutex_;
    
    // Protocol instance for message creation
    dtc::Protocol protocol_;
};

} // namespace server
} // namespace coinbase_dtc_core
