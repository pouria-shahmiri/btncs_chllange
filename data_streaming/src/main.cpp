#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <chrono>

#include "MBOParsed.hpp"
#include "MBOPublisher.hpp"

MBOParsed parseCSVLine(const std::string& line) {
    MBOParsed record{};
    std::stringstream ss(line);
    std::string field;
    std::vector<std::string> fields;
    
    // Split by comma
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    
    if (fields.size() >= 15) {
        record.ts_event_str = fields[0];
        record.rtype = static_cast<uint8_t>(std::stoi(fields[1]));
        record.publisher_id = static_cast<uint16_t>(std::stoi(fields[2]));
        record.instrument_id = static_cast<uint32_t>(std::stoul(fields[3]));
        record.action = fields[4][0];
        record.side = fields[5][0];
        record.price = std::stod(fields[6]);
        record.size = static_cast<uint32_t>(std::stoul(fields[7]));
        record.channel_id = static_cast<uint8_t>(std::stoi(fields[8]));
        record.order_id = std::stoull(fields[9]);
        record.flags = static_cast<uint8_t>(std::stoi(fields[10]));
        record.ts_in_delta = std::stoi(fields[11]);
        record.sequence = static_cast<uint32_t>(std::stoul(fields[12]));
        record.symbol = fields[13];
        record.datetime = fields[14];
    }
    
    return record;
}

void printRecord(const MBOParsed& r) {
    std::cout << std::fixed << std::setprecision(2)
              << "ts_event=" << r.ts_event_str
              << " rtype=" << int(r.rtype)
              << " publisher_id=" << r.publisher_id
              << " instrument_id=" << r.instrument_id
              << " action=" << r.action
              << " side=" << r.side
              << " price=" << r.price
              << " size=" << r.size
              << " channel_id=" << int(r.channel_id)
              << " order_id=" << r.order_id
              << " flags=" << int(r.flags)
              << " ts_in_delta=" << r.ts_in_delta
              << " sequence=" << r.sequence
              << " symbol=" << r.symbol
              << " datetime=" << r.datetime
              << std::endl;
}


int main() {
    try {
        std::ifstream file("./data.csv");
        if (!file.is_open()) {
            std::cerr << "Error: Could not open data.csv" << std::endl;
            return 1;
        }

        std::string line;
        std::vector<MBOParsed> records;

        std::getline(file, line); // skip header
        while (std::getline(file, line)) {
            if (!line.empty()) {
                records.push_back(parseCSVLine(line));
            }
        }
        file.close();

        std::cout << "Total records loaded: " << records.size() << std::endl;

        // Init DDS publisher
        MBOPublisher publisher;
        if (!publisher.init()) {
            std::cerr << "Failed to initialize DDS publisher" << std::endl;
            return 1;
        }

        size_t i = 0;
        while (true) {  // infinite replay loop
            if (records[i].ts_in_delta > 0) {
                std::cout << "Sleeping for " << records[i].ts_in_delta << " microseconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::microseconds(records[i].ts_in_delta));
            }

            printRecord(records[i]);
            publisher.publish(records[i]);

            i++;
            if (i >= records.size()) {
                std::cout << "\nReached end of records — restarting from beginning.\n" << std::endl;
                i = 0; // restart from first record
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}



