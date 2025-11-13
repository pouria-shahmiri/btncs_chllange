#include <iostream>
#include "MBOSubscriber.hpp"

int main() {
    std::cout << "=== MBO Order Book Subscriber ===" << std::endl;
    
    MBOSubscriber subscriber;
    
    if (!subscriber.init()) {
        std::cerr << "Failed to initialize subscriber" << std::endl;
        return 1;
    }
    
    std::cout << "Subscriber ready. Listening for MBO data..." << std::endl;
    
    try {
        subscriber.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}