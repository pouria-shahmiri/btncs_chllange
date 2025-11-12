#include "dbn_reader.h"
#include "market_data_publisher.h"
#include <iostream>
#include <thread>
#include <chrono>




int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <dbn_file_path>" << std::endl;
        return 1;
    }

    std::string dbnFile = argv[1];
    DBNReader reader(dbnFile);
    MarketDataPublisher publisher;

    // Initialize FastDDS publisher
    if (!publisher.initialize()) {
        std::cerr << "Failed to initialize FastDDS publisher" << std::endl;
        return 1;
    }
    
    // Give time for discovery
    std::cout << "\nWaiting for subscriber discovery..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Open and read DBN file
    if (!reader.open()) {
        std::cerr << "Failed to open DBN file" << std::endl;
        return 1;
    }

    // Print header information
    reader.printHeader();

    // Read and print all records
    MBORecord record;
    int recordCount = 0;

    std::cout << "Reading records from DBN file..." << std::endl;
    std::cout << "=== Records ===" << std::endl;

    // Loop infinitely, restarting from beginning when reaching end
    while (true) {
        if (reader.readNextRecord(record)) {
            reader.printRecord(record);
            publisher.publishRecord(record);  // Publish to FastDDS
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small delay to allow processing
            recordCount++;
        } else {
            // Reached end of file, restart from beginning
            std::cout << "\n--- Reached end of file, restarting from beginning ---\n" << std::endl;
            reader.close();
            if (!reader.open()) {
                std::cerr << "Failed to reopen DBN file" << std::endl;
                break;
            }
            // Skip header
            reader.getHeader();
        }
    }

    reader.close();
    publisher.shutdown();

    std::cout << "=================" << std::endl;
    std::cout << "Total records read: " << recordCount << std::endl;

    return 0;
}
