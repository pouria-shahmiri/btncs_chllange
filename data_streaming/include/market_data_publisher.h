#ifndef MARKET_DATA_PUBLISHER_H
#define MARKET_DATA_PUBLISHER_H

#include "dbn_reader.h"

class MarketDataPublisher {
public:
    MarketDataPublisher();
    ~MarketDataPublisher();
    
    bool initialize();
    void publishRecord(const MBORecord& record);
    void shutdown();
    
private:
    void* participant_;
    void* publisher_;
    void* topic_;
    void* writer_;
    // TCP server socket to stream the same published text to a subscriber
    int server_fd_;
    int client_fd_;
};

#endif // MARKET_DATA_PUBLISHER_H
