#include "orderbook_subscriber.h"
#include <iostream>
#include <signal.h>

OrderbookSubscriber* g_subscriber = nullptr;

void signalHandler(int signum) {
    std::cout << "\n\nReceived interrupt signal (" << signum << ")" << std::endl;
    if (g_subscriber) {
        g_subscriber->shutdown();
    }
    exit(signum);
}

int main() {
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║     Recon Orderbook - Market Data Subscriber    ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝\n" << std::endl;
    
    OrderbookSubscriber subscriber;
    g_subscriber = &subscriber;
    
    // Initialize the subscriber
    if (!subscriber.initialize()) {
        std::cerr << "Failed to initialize subscriber" << std::endl;
        return 1;
    }
    
    // Start listening for data
    subscriber.listen();
    
    std::cout << "\nApplication terminated." << std::endl;
    return 0;
}
