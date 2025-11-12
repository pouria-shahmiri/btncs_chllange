#ifndef DBN_READER_H
#define DBN_READER_H

#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>

#pragma pack(push, 1)

// DBN Header Structure
struct DBNHeader {
    uint32_t magic;           // "DBN\x03"
    uint16_t version;
    uint16_t flags;
    char schema[8];
    char dataset[32];
    uint64_t start_ts;
    uint64_t end_ts;
    uint32_t stype_in;
    uint32_t stype_out;
    char symbols[256];
    char partial[1024];
    char not_found[1024];
};

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

class DBNReader {
public:
    explicit DBNReader(const std::string& filename);
    ~DBNReader();
    
    bool open();
    void close();
    
    const DBNHeader& getHeader() const { return header; }
    
    bool readNextRecord(MBORecord& record);
    void printHeader() const;
    void printRecord(const MBORecord& record) const;
    
private:
    std::string filename;
    std::ifstream file;
    DBNHeader header;
    bool isOpen;
};

#endif // DBN_READER_H
