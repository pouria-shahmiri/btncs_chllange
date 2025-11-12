# 📋 Project Index & File Reference

## 🎯 Quick Links

- **[README.md](README.md)** - Start here for overview
- **[TEST_GUIDE.md](TEST_GUIDE.md)** - Complete testing instructions
- **[SUMMARY.md](SUMMARY.md)** - Comprehensive project summary
- **[QUICK_START.sh](QUICK_START.sh)** - Quick reference guide

## 📁 Directory Structure

```
data_streaming/
├── 📚 Documentation
│   ├── README.md              ← Project overview & architecture
│   ├── TEST_GUIDE.md          ← Testing instructions & examples
│   ├── SUMMARY.md             ← Complete project summary
│   ├── INDEX.md               ← This file
│   └── QUICK_START.sh         ← Quick reference
│
├── 🔨 Build System
│   ├── Makefile               ← Main build file (use this!)
│   └── CMakeLists.txt         ← Alternative CMake config
│
├── 📂 include/ (Header Files)
│   ├── dbn_reader.h           ← DBN file format definitions & parser
│   ├── market_data_publisher.h ← FastDDS publisher interface
│   └── market_data_subscriber.h ← FastDDS subscriber interface
│
├── 📂 src/ (Source Files)
│   ├── main.cpp               ← Publisher main application
│   ├── subscriber_test.cpp    ← Subscriber main application
│   ├── dbn_reader.cpp         ← Binary .dbn file reader
│   ├── market_data_publisher.cpp  ← Publisher implementation
│   └── market_data_subscriber.cpp ← Subscriber implementation
│
├── 📂 build/ (Auto-generated)
│   ├── main.o                 ← Compiled objects
│   ├── dbn_reader.o
│   ├── market_data_publisher.o
│   ├── subscriber_test.o
│   └── market_data_subscriber.o
│
├── 🚀 Executables (Built Binaries)
│   ├── dbn_reader (37KB)      ← Publisher executable
│   └── market_data_subscriber (21KB) ← Subscriber executable
│
└── 📊 Data (External)
    └── ../CLX5_mbo (2).dbn    ← Market data file (2.1MB)
```

## 📖 File Descriptions

### Documentation Files

| File | Purpose | Read When |
|------|---------|-----------|
| **README.md** | Complete documentation | Starting the project |
| **TEST_GUIDE.md** | Detailed test instructions | Running tests |
| **SUMMARY.md** | Comprehensive summary | Need full details |
| **INDEX.md** | This file | Navigating the project |
| **QUICK_START.sh** | Quick commands | Need quick commands |

### Build Configuration

| File | Purpose |
|------|---------|
| **Makefile** | Primary build tool (recommended) |
| **CMakeLists.txt** | CMake alternative build config |

### Header Files (include/)

| File | Classes | Key Functions |
|------|---------|---------------|
| **dbn_reader.h** | DBNHeader, MBORecord, DBNReader | open(), readNextRecord(), printRecord() |
| **market_data_publisher.h** | MarketDataPublisher | initialize(), publishRecord(), shutdown() |
| **market_data_subscriber.h** | MarketDataSubscriber | initialize(), listen(), shutdown() |

### Source Files (src/)

| File | Type | Lines | Purpose |
|------|------|-------|---------|
| **main.cpp** | Publisher | ~65 | Main publisher application entry point |
| **subscriber_test.cpp** | Subscriber | ~50 | Main subscriber application entry point |
| **dbn_reader.cpp** | Core | ~75 | Binary .dbn file parsing logic |
| **market_data_publisher.cpp** | Core | ~25 | FastDDS publisher implementation |
| **market_data_subscriber.cpp** | Core | ~40 | FastDDS subscriber implementation |

## 🚀 Common Commands

### Build
```bash
cd /home/pouria/project/batonics/data_streaming
make clean && make all    # Build everything
```

### Run
```bash
make run                  # Run publisher
make run_subscriber       # Run subscriber
```

### Clean
```bash
make clean               # Remove build artifacts
```

## 🧠 Understanding the Code

### Data Flow
```
1. .dbn File (2.1MB)
   ↓
2. dbn_reader (reads binary)
   ↓
3. Parse MBORecord structures
   ↓
4. FastDDS Publisher publishes
   ↓
5. FastDDS "MarketData" topic
   ↓
6. market_data_subscriber listens
   ↓
7. Display received messages
```

### Key Classes

**DBNReader** (dbn_reader.h/cpp)
```cpp
class DBNReader {
    bool open()              // Open .dbn file
    bool readNextRecord()    // Read next record
    void printRecord()       // Display record
    void printHeader()       // Display header info
};
```

**MarketDataPublisher** (market_data_publisher.h/cpp)
```cpp
class MarketDataPublisher {
    bool initialize()        // Initialize FastDDS
    void publishRecord()     // Publish a record
    void shutdown()          // Clean shutdown
};
```

**MarketDataSubscriber** (market_data_subscriber.h/cpp)
```cpp
class MarketDataSubscriber {
    bool initialize()        // Initialize FastDDS
    void listen()            // Listen for messages
    void shutdown()          // Clean shutdown
};
```

## 📊 Record Structure

Each market data record contains:
```
ts_event    : uint64_t  (timestamp in nanoseconds)
rtype       : uint8_t   (record type)
publisher_id: uint16_t  (publisher ID)
instrument_id: uint32_t (instrument ID)
action      : uint8_t   (A/C/etc)
side        : uint8_t   (Ask/Bid)
price       : uint32_t  (price value)
size        : uint32_t  (order size)
channel_id  : uint16_t  (channel ID)
order_id    : uint64_t  (unique order ID)
flags       : uint8_t   (bitflags)
ts_in_delta : uint16_t  (timestamp delta)
sequence    : uint64_t  (sequence number)
symbol      : char[8]   (symbol: "CLX5")
```

## 🧪 Testing Checklist

- [ ] Run `make clean && make all`
- [ ] Check binaries exist: `dbn_reader` and `market_data_subscriber`
- [ ] Terminal 1: `./dbn_reader ../CLX5_mbo\ \(2\).dbn`
- [ ] Terminal 2: `./market_data_subscriber`
- [ ] See "[DDS] Publishing" messages
- [ ] See "[Subscriber] Listening" messages
- [ ] Press Ctrl+C to stop
- [ ] Both exit gracefully

## 🔗 Related Files

- `.dbn Data File`: `../CLX5_mbo (2).dbn` (2.1MB)
- `Python Reader`: `../data_analyze/main.py` (Databento Python library reference)

## 💡 Tips

1. **Before running:** Make sure you're in `/home/pouria/project/batonics/data_streaming`
2. **Use Makefile:** `make run` is easier than manual compilation
3. **Two terminals:** Always run publisher and subscriber in separate terminals
4. **Graceful stop:** Use Ctrl+C to stop either application
5. **Restart:** Just run the command again - no cleanup needed

## ⚡ Performance Notes

- **File:** 2.1 MB
- **Message Rate:** 20 messages/second
- **Interval:** 50 milliseconds
- **Loop:** Infinite (restarts at end)
- **Protocol:** FastDDS/TCP
- **Language:** C++17

## 🎓 Learning Resources

- **FastDDS Docs:** https://fast-dds.docs.eprosima.com/
- **Databento DBN Format:** Check Python reader in `../data_analyze/main.py`
- **C++ Resources:** Standard C++17 library documentation

## ✅ Project Status

| Component | Status |
|-----------|--------|
| Publisher | ✅ Complete |
| Subscriber | ✅ Complete |
| Build System | ✅ Complete |
| Documentation | ✅ Complete |
| Testing | ✅ Tested |
| Ready for Use | ✅ YES |

---

**Last Updated:** November 12, 2025  
**Status:** Ready for production use 🚀
