#include "market_data_subscriber.h"
#include <iostream>
#include <signal.h>

MarketDataSubscriber* g_subscriber = nullptr;

void signalHandler(int signum) {
    std::cout << "\n\nReceived interrupt signal (" << signum << "). Shutting down..." << std::endl;
    if (g_subscriber) {
        g_subscriber->shutdown();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    // Silence unused-parameter warnings when argc/argv aren't used
    (void)argc;
    (void)argv;
    std::cout << "=== Market Data FastDDS Subscriber ===" << std::endl;
    std::cout << "This subscriber listens for market data published via FastDDS" << std::endl << std::endl;
    
    // Register signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    MarketDataSubscriber subscriber;
    g_subscriber = &subscriber;
    
    // Initialize subscriber
    if (!subscriber.initialize()) {
        std::cerr << "Failed to initialize subscriber" << std::endl;
        return 1;
    }
    
    // Start listening for market data
    subscriber.listen();
    
    subscriber.shutdown();
    
    return 0;
}
