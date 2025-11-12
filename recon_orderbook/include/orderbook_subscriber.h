#ifndef ORDERBOOK_SUBSCRIBER_H
#define ORDERBOOK_SUBSCRIBER_H

#include <cstdint>
#include <string>

#pragma pack(push, 1)

// MBO (Market By Order) Record Structure
struct MBORecord {
    uint64_t ts_event;        // Timestamp event (nanoseconds)
    uint8_t rtype;            // Record type
    uint16_t publisher_id;    // Publisher ID
    uint32_t instrument_id;   // Instrument ID
    uint8_t action;           // Action (A=Add, C=Cancel, etc.)
    uint8_t side;             // Side (A=Ask, B=Bid)
    uint32_t price;           // Price (fixed point)
    uint32_t size;            // Size
    uint16_t channel_id;      // Channel ID
    uint64_t order_id;        // Order ID
    uint8_t flags;            // Flags
    uint16_t ts_in_delta;     // Timestamp in delta
    uint64_t sequence;        // Sequence number
    char symbol[8];           // Symbol (null-terminated)
};

#pragma pack(pop)

class OrderbookSubscriber {
public:
    OrderbookSubscriber();
    ~OrderbookSubscriber();
    
    // Initialize subscriber
    bool initialize();
    
    // Start listening for data
    void listen();
    
    // Shutdown subscriber
    void shutdown();
    
private:
    void* participant_;
    void* subscriber_;
    void* topic_;
    void* reader_;
    bool running_;
    uint64_t recordCount_;
};

#endif // ORDERBOOK_SUBSCRIBER_H
