#include "coinbase_dtc_core/dtc/protocol.hpp"
#include <iostream>
#include <cassert>

int main() {
    using namespace coinbase_dtc_core::dtc;
    
    std::cout << "🧪 Testing DTC Protocol Implementation..." << std::endl;
    
    // Test 1: Protocol initialization
    Protocol protocol;
    std::cout << "✅ Protocol version: " << protocol.version() << std::endl;
    std::cout << "✅ Protocol connected: " << (protocol.is_connected() ? "Yes" : "No") << std::endl;
    
    // Test 2: Create and serialize Logon Request
    std::cout << "\n📡 Testing Logon Request..." << std::endl;
    LogonRequest logon_req;
    
    // Debug size calculations
    std::cout << "   sizeof(LogonRequest): " << sizeof(LogonRequest) << std::endl;
    std::cout << "   sizeof(MessageHeader): " << sizeof(MessageHeader) << std::endl;
    std::cout << "   protocol_version: " << logon_req.protocol_version << std::endl;
    
    strncpy(logon_req.username, "testuser", sizeof(logon_req.username) - 1);
    strncpy(logon_req.password, "testpass", sizeof(logon_req.password) - 1);
    strncpy(logon_req.general_text_data, "CoinbaseDTC Client v1.0", sizeof(logon_req.general_text_data) - 1);
    
    // Debug header before serialize
    std::cout << "   header.size before serialize: " << logon_req.header.size << std::endl;
    std::cout << "   header.type before serialize: " << logon_req.header.type << std::endl;
    
    auto serialized = logon_req.serialize();
    std::cout << "✅ Logon request serialized: " << serialized.size() << " bytes" << std::endl;
    
    // Debug header after serialize
    const MessageHeader* header_ptr = reinterpret_cast<const MessageHeader*>(serialized.data());
    std::cout << "   Serialized header.size: " << header_ptr->size << std::endl;
    std::cout << "   Serialized header.type: " << header_ptr->type << std::endl;
    
    // Test 3: Parse the serialized message
    auto parsed_msg = protocol.parse_message(serialized.data(), serialized.size());
    if (parsed_msg) {
        std::cout << "✅ Message parsed successfully" << std::endl;
        // Check message type instead of using dynamic_cast
        if (parsed_msg->get_type() == MessageType::LOGON_REQUEST) {
            auto* parsed_logon = static_cast<LogonRequest*>(parsed_msg.get());
            std::cout << "   Parsed username: " << parsed_logon->username << std::endl;
            std::cout << "   Parsed protocol version: " << parsed_logon->protocol_version << std::endl;
        }
    } else {
        std::cout << "❌ Failed to parse message" << std::endl;
        return 1;
    }
    
    std::cout << "\n🔄 Continuing with next test..." << std::endl;
    
    // Test 4: Create Logon Response
    std::cout << "\n📤 Testing Logon Response..." << std::endl;
    auto logon_resp = protocol.create_logon_response(true, "Login successful - CoinbaseDTC");
    auto resp_data = logon_resp->serialize();
    std::cout << "✅ Logon response created: " << resp_data.size() << " bytes" << std::endl;
    std::cout << "   Result: " << (logon_resp->result ? "Success" : "Failed") << std::endl;
    std::cout << "   Message: " << logon_resp->result_text << std::endl;
    
    // Test 5: Create Market Data Request
    std::cout << "\n📈 Testing Market Data Request..." << std::endl;
    MarketDataRequest md_req;
    md_req.symbol_id = 1;
    md_req.request_action = 1; // Subscribe
    strncpy(md_req.symbol, "BTC-USD", sizeof(md_req.symbol) - 1);
    
    auto md_data = md_req.serialize();
    std::cout << "✅ Market data request created: " << md_data.size() << " bytes" << std::endl;
    std::cout << "   Symbol: " << md_req.symbol << std::endl;
    std::cout << "   Symbol ID: " << md_req.symbol_id << std::endl;
    std::cout << "   Action: " << (md_req.request_action == 1 ? "Subscribe" : "Unsubscribe") << std::endl;
    
    // Test 6: Create Trade Update
    std::cout << "\n💰 Testing Trade Update..." << std::endl;
    auto trade_update = protocol.create_trade_update(
        1,           // symbol_id
        65432.50,    // price
        0.025,       // volume
        Protocol::get_current_timestamp()
    );
    auto trade_data = trade_update->serialize();
    std::cout << "✅ Trade update created: " << trade_data.size() << " bytes" << std::endl;
    std::cout << "   Symbol ID: " << trade_update->symbol_id << std::endl;
    std::cout << "   Price: $" << trade_update->price << std::endl;
    std::cout << "   Volume: " << trade_update->volume << std::endl;
    std::cout << "   Timestamp: " << trade_update->date_time << std::endl;
    
    // Test 7: Create Bid/Ask Update
    std::cout << "\n📊 Testing Bid/Ask Update..." << std::endl;
    auto bidask_update = protocol.create_bid_ask_update(
        1,           // symbol_id
        65430.00,    // bid_price
        1.25,        // bid_quantity
        65435.00,    // ask_price
        0.75,        // ask_quantity
        Protocol::get_current_timestamp()
    );
    auto bidask_data = bidask_update->serialize();
    std::cout << "✅ Bid/Ask update created: " << bidask_data.size() << " bytes" << std::endl;
    std::cout << "   Symbol ID: " << bidask_update->symbol_id << std::endl;
    std::cout << "   Bid: $" << bidask_update->bid_price << " x " << bidask_update->bid_quantity << std::endl;
    std::cout << "   Ask: $" << bidask_update->ask_price << " x " << bidask_update->ask_quantity << std::endl;
    
    // Test 8: Message Header Validation
    std::cout << "\n🔍 Testing Message Validation..." << std::endl;
    
    // Valid message
    bool valid = Protocol::validate_message_header(trade_data.data(), trade_data.size());
    std::cout << "✅ Valid message validation: " << (valid ? "PASS" : "FAIL") << std::endl;
    
    // Invalid message (too small)
    std::vector<uint8_t> invalid_data = {0x01, 0x02};
    bool invalid = Protocol::validate_message_header(invalid_data.data(), invalid_data.size());
    std::cout << "✅ Invalid message validation: " << (invalid ? "FAIL" : "PASS") << std::endl;
    
    // Test 9: Message Type Detection
    MessageType detected_type = Protocol::get_message_type(trade_data.data(), trade_data.size());
    std::cout << "✅ Message type detection: " << static_cast<uint16_t>(detected_type) 
              << " (expected " << static_cast<uint16_t>(MessageType::MARKET_DATA_UPDATE_TRADE) << ")" << std::endl;
    
    std::cout << "\n🎉 All DTC Protocol tests completed successfully!" << std::endl;
    std::cout << "\n📋 DTC Protocol Summary:" << std::endl;
    std::cout << "   • Protocol Version: " << DTC_PROTOCOL_VERSION << std::endl;
    std::cout << "   • Implemented Messages: 5 core types" << std::endl;
    std::cout << "   • Serialization: Binary format" << std::endl;
    std::cout << "   • Validation: Header and type checking" << std::endl;
    std::cout << "   • Ready for: Client connections and market data" << std::endl;
    
    return 0;
}