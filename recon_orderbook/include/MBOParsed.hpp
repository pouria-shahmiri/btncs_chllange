#pragma once
#include <string>
#include <cstdint>

struct MBOParsed {
    std::string ts_event_str;
    uint8_t  rtype;
    uint16_t publisher_id;
    uint32_t instrument_id;
    char     action;
    char     side;
    double   price;
    uint32_t size;
    uint8_t  channel_id;
    uint64_t order_id;
    uint8_t  flags;
    int32_t  ts_in_delta;
    uint32_t sequence;
    std::string symbol;
    std::string datetime;
};