#ifndef MARKET_DATA_SUBSCRIBER_H
#define MARKET_DATA_SUBSCRIBER_H

#include "dbn_reader.h"
#include <cstdint>

/*
 * FastDDS Subscriber for Market Data
 * 
 * How FastDDS Discovers and Connects:
 * ===================================
 * 1. DISCOVERY PHASE:
 *    - Subscriber and Publisher join same Domain (0)
 *    - Simple Discovery Protocol (SDP) is used
 *    - Both announce their topics to RTPS discovery service
 *    - Discovery port: 7400-7410 (UDP)
 *
 * 2. MATCHING PHASE:
 *    - DDS discovers when Topic Names match ("MarketData")
 *    - QoS policies are checked for compatibility
 *    - Built-in endpoints exchange metadata
 *    - RTPS connection (port 11511) is established
 *
 * 3. DATA TRANSPORT:
 *    - TCP connection established between matched endpoints
 *    - No manual connection string needed
 *    - Automatic failover and reconnection
 *    - Data streamed via FastDDS RTPS protocol
 *
 * KEY POINTS:
 * - Domain ID must match (0 in this case)
 * - Topic Name must match ("MarketData")
 * - QoS must be compatible (reliability, history policy)
 * - Default port range: 7400-7410 (discovery), 11511 (RTPS)
 * - Multicast or unicast discovery possible
 * - No hardcoded IP addresses needed!
 */

class MarketDataSubscriber {
public:
    MarketDataSubscriber();
    ~MarketDataSubscriber();
    
    // Initialize FastDDS subscriber with discovery configuration
    bool initialize();
    
    // Listen for incoming market data messages with latency tracking
    void listen();
    
    // Graceful shutdown
    void shutdown();
    
private:
    void* participant_;      // FastDDS DomainParticipant
    void* subscriber_;       // FastDDS Subscriber
    void* topic_;           // FastDDS Topic ("MarketData")
    void* reader_;          // FastDDS DataReader
};

#endif // MARKET_DATA_SUBSCRIBER_H
