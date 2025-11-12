#include "dbn_reader.h"
#include <iostream>
#include <iomanip>

DBNReader::DBNReader(const std::string& filename)
    : filename(filename), isOpen(false) {
    std::memset(&header, 0, sizeof(header));
}

DBNReader::~DBNReader() {
    close();
}

bool DBNReader::open() {
    file.open(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    // Read header
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (file.fail()) {
        std::cerr << "Error: Cannot read header from file" << std::endl;
        file.close();
        return false;
    }

    isOpen = true;
    return true;
}

void DBNReader::close() {
    if (file.is_open()) {
        file.close();
    }
    isOpen = false;
}

bool DBNReader::readNextRecord(MBORecord& record) {
    if (!isOpen) {
        return false;
    }

    file.read(reinterpret_cast<char*>(&record), sizeof(record));
    
    if (file.eof()) {
        return false;
    }
    
    if (file.fail()) {
        std::cerr << "Error reading record" << std::endl;
        return false;
    }

    return true;
}

void DBNReader::printHeader() const {
    std::cout << "=== DBN Header ===" << std::endl;
    std::cout << "Magic: ";
    for (int i = 0; i < 4; i++) {
        if (i < 3) std::cout << header.schema[i];
        else std::cout << (int)header.schema[i];
    }
    std::cout << std::endl;
    std::cout << "Version: " << header.version << std::endl;
    std::cout << "Flags: " << header.flags << std::endl;
    std::cout << "Schema: " << std::string(header.schema, strnlen(header.schema, 8)) << std::endl;
    std::cout << "Dataset: " << std::string(header.dataset, strnlen(header.dataset, 32)) << std::endl;
    std::cout << "Start Timestamp: " << header.start_ts << std::endl;
    std::cout << "End Timestamp: " << header.end_ts << std::endl;
    std::cout << "=================" << std::endl << std::endl;
}

void DBNReader::printRecord(const MBORecord& record) const {
    std::cout << "ts_event: " << std::setw(20) << record.ts_event
              << " | rtype: " << std::setw(3) << (int)record.rtype
              << " | publisher_id: " << std::setw(5) << record.publisher_id
              << " | instrument_id: " << std::setw(8) << record.instrument_id
              << " | action: " << record.action
              << " | side: " << record.side
              << " | price: " << std::setw(8) << record.price
              << " | size: " << std::setw(5) << record.size
              << " | channel_id: " << std::setw(3) << record.channel_id
              << " | order_id: " << std::setw(15) << record.order_id
              << " | flags: " << std::setw(3) << (int)record.flags
              << " | ts_in_delta: " << std::setw(5) << record.ts_in_delta
              << " | sequence: " << std::setw(10) << record.sequence
              << " | symbol: " << std::string(record.symbol, strnlen(record.symbol, 8))
              << std::endl;
}
