# Market Data FastDDS Publisher/Subscriber Test

## Overview
This is a complete C++ FastDDS application with a publisher that reads .dbn (Databento) market data files and publishes them, and a subscriber that listens to these published messages.

## Project Structure

```
data_streaming/
├── include/
│   ├── dbn_reader.h                 # DBN file reader header
│   ├── market_data_publisher.h      # FastDDS publisher header
│   └── market_data_subscriber.h     # FastDDS subscriber header
├── src/
│   ├── main.cpp                     # Publisher main application
│   ├── dbn_reader.cpp               # DBN file reader implementation
│   ├── market_data_publisher.cpp    # FastDDS publisher implementation
│   ├── subscriber_test.cpp          # Subscriber main application
│   └── market_data_subscriber.cpp   # FastDDS subscriber implementation
├── Makefile                         # Build configuration
├── CMakeLists.txt                  # CMake configuration
└── README.md                        # This file
```

## Building

### Build All (Publisher + Subscriber)
```bash
cd /home/pouria/project/batonics/data_streaming
make clean
make all
```

### Build Only Publisher
```bash
make dbn_reader
```

### Build Only Subscriber
```bash
make market_data_subscriber
```

## Running the Applications

### Option 1: Publisher and Subscriber in Separate Terminals

**Terminal 1 - Start Publisher:**
```bash
cd /home/pouria/project/batonics/data_streaming
./dbn_reader ../CLX5_mbo\ \(2\).dbn
```

**Terminal 2 - Start Subscriber:**
```bash
cd /home/pouria/project/batonics/data_streaming
./market_data_subscriber
```

### Option 2: Publisher in Background, Subscriber in Foreground
```bash
cd /home/pouria/project/batonics/data_streaming
./dbn_reader ../CLX5_mbo\ \(2\).dbn &
./market_data_subscriber
```

### Option 3: Use Makefile Commands
```bash
cd /home/pouria/project/batonics/data_streaming
make run                    # Run publisher only
make run_subscriber         # Run subscriber only
make run_both              # Show instructions for running both
```

## Application Behavior

### Publisher (dbn_reader)
- Reads the binary .dbn file
- Parses market data records with the following fields:
  - `ts_event`: Event timestamp (nanoseconds)
  - `rtype`: Record type
  - `publisher_id`: Publisher ID
  - `instrument_id`: Instrument ID
  - `action`: Action (Add, Cancel, etc.)
  - `side`: Side (Ask/Bid)
  - `price`: Price
  - `size`: Order size
  - `channel_id`: Channel ID
  - `order_id`: Order ID
  - `flags`: Various flags
  - `ts_in_delta`: Timestamp delta
  - `sequence`: Sequence number
  - `symbol`: Trading symbol
- Publishes each record to FastDDS topic "MarketData"
- Loops infinitely, restarting from beginning when reaching end of file

### Subscriber (market_data_subscriber)
- Initializes FastDDS subscriber
- Listens to "MarketData" topic
- Receives and processes market data records published by the publisher
- Prints status messages showing reception
- Exits gracefully on SIGINT (Ctrl+C)

## Data Flow

```
DBN File
   ↓
dbn_reader (Publisher)
   ├─→ Parse binary records
   ├─→ Publish via FastDDS
   └─→ Continuous loop (restart at end)
   
FastDDS Topic: "MarketData"
   ↓
market_data_subscriber (Subscriber)
   ├─→ Listen for messages
   ├─→ Process records
   └─→ Display status
```

## Message Format

Each published message contains:
```
ts_event|price|size|side|order_id|sequence
```

Example:
```
[DDS] Publishing: 1858299205099534|1414003041|7|222|1099511631643|5671872943036383555
```

## Troubleshooting

### Build Errors
- Ensure FastDDS development libraries are installed:
  ```bash
  sudo apt-get install libfastrtps-dev libfastcdr-dev
  ```

### Runtime Issues
- Both publisher and subscriber must run to establish FastDDS communication
- They can run on the same machine or different machines if networked properly
- Check firewall settings if running on different machines

### Graceful Shutdown
- Press `Ctrl+C` to stop either application gracefully
- The subscriber will print total message count before exiting

## Performance Notes
- Publisher reads file sequentially and publishes at 50ms intervals
- Subscriber listens continuously and displays status every 5 seconds (50 * 100 iterations)
- Infinite loop allows continuous testing and monitoring

## Dependencies
- FastDDS 2.11.2
- FastCDR 1.1.0
- Standard C++ 17
- GCC/G++ compiler

## Author
Generated for Databento market data analysis

## License
MIT
