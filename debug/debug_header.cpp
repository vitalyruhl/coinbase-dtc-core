#include <iostream>
#include <iomanip>
#include <cstdint>

#pragma pack(push, 1)
struct MessageHeader {
    uint16_t size;        
    uint16_t type;        
    
    MessageHeader() : size(0), type(0) {}
    MessageHeader(uint16_t msg_size, uint16_t msg_type) 
        : size(msg_size), type(msg_type) {}
};
#pragma pack(pop)

int main() {
    std::cout << "MessageHeader size: " << sizeof(MessageHeader) << " bytes" << std::endl;
    
    MessageHeader header(272, 1);
    std::cout << "Created header - size: " << header.size << ", type: " << header.type << std::endl;
    
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&header);
    std::cout << "Header bytes: ";
    for (int i = 0; i < sizeof(MessageHeader); ++i) {
        std::printf("%02X ", bytes[i]);
    }
    std::cout << std::endl;
    
    return 0;
}
