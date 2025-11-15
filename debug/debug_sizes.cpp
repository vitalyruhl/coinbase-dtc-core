#include "coinbase_dtc_core/dtc/protocol.hpp"
#include <iostream>

int main() {
    using namespace coinbase_dtc_core::dtc;
    
    std::cout << "DTC Message Struct Sizes:" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << "MessageHeader: " << sizeof(MessageHeader) << " bytes" << std::endl;
    std::cout << "LogonRequest: " << sizeof(LogonRequest) << " bytes" << std::endl;
    std::cout << "LogonResponse: " << sizeof(LogonResponse) << " bytes" << std::endl;
    std::cout << "MarketDataRequest: " << sizeof(MarketDataRequest) << " bytes" << std::endl;
    std::cout << "MarketDataUpdateTrade: " << sizeof(MarketDataUpdateTrade) << " bytes" << std::endl;
    std::cout << "MarketDataUpdateBidAsk: " << sizeof(MarketDataUpdateBidAsk) << " bytes" << std::endl;
    
    // Check calculated sizes
    std::cout << "\nCalculated Sizes (without vtable):" << std::endl;
    std::cout << "====================================" << std::endl;
    
    MarketDataUpdateBidAsk bid_ask_msg;
    std::cout << "MarketDataUpdateBidAsk calculated size: " << bid_ask_msg.get_size() << " bytes" << std::endl;
    std::cout << "MarketDataUpdateBidAsk header size: " << bid_ask_msg.header.size << " bytes" << std::endl;
    
    MarketDataUpdateTrade trade_msg;
    std::cout << "MarketDataUpdateTrade calculated size: " << trade_msg.get_size() << " bytes" << std::endl;
    std::cout << "MarketDataUpdateTrade header size: " << trade_msg.header.size << " bytes" << std::endl;
    
    std::cout << "\nField sizes in MarketDataUpdateBidAsk:" << std::endl;
    std::cout << "symbol_id: " << sizeof(bid_ask_msg.symbol_id) << " bytes" << std::endl;
    std::cout << "bid_price: " << sizeof(bid_ask_msg.bid_price) << " bytes" << std::endl;
    std::cout << "bid_quantity: " << sizeof(bid_ask_msg.bid_quantity) << " bytes" << std::endl;
    std::cout << "ask_price: " << sizeof(bid_ask_msg.ask_price) << " bytes" << std::endl;
    std::cout << "ask_quantity: " << sizeof(bid_ask_msg.ask_quantity) << " bytes" << std::endl;
    std::cout << "date_time: " << sizeof(bid_ask_msg.date_time) << " bytes" << std::endl;
    
    return 0;
}
