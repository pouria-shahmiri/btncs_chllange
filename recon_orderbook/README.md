# Recon Orderbook - Market Data Subscriber

A C++ FastDDS subscriber that listens to market data published by the `data_streaming` publisher and logs all incoming MBO (Market By Order) records to the console.

## Project Structure

```
recon_orderbook/
├── Makefile                  # Build configuration
├── include/
│   └── orderbook_subscriber.h    # Subscriber header with MBO record structure
├── src/
│   ├── main.cpp              # Application entry point
│   └── orderbook_subscriber.cpp  # Subscriber implementation with FastDDS integration
└── build/                    # Build output directory
```

## Features

- ✅ Real FastDDS communication with the data_streaming publisher
- ✅ Subscribes to "MarketData" topic with automatic discovery
- ✅ Logs all incoming MBO records to console in real-time
- ✅ Graceful shutdown handling (Ctrl+C)
- ✅ Simple and clean makefile

## Building

### Build the Project
```bash
cd /home/pouria/project/batonics/recon_orderbook
make clean
make
```

### Build Options
```bash
make           # Build the subscriber
make run       # Clean, build, and run
make clean     # Remove build artifacts
make help      # Show available targets
```

## Running

### Prerequisite: Start the Publisher
The subscriber requires the `data_streaming` publisher to be running first. In a separate terminal:

```bash
cd /home/pouria/project/batonics/data_streaming
./dbn_reader ../data_analyze/CLX5_mbo\ \(2\).dbn
```

### Run the Subscriber
```bash
cd /home/pouria/project/batonics/recon_orderbook
./recon_orderbook
```

### Quick Start (Build and Run)
```bash
cd /home/pouria/project/batonics/recon_orderbook
make run
```

## How It Works

1. **Initialization Phase**: The subscriber creates a FastDDS DomainParticipant and joins domain 0
2. **Discovery Phase**: FastDDS Simple Discovery Protocol discovers the publisher on the "MarketData" topic
3. **Connection Phase**: Automatic TCP connection established between subscriber and publisher
4. **Data Reception**: All MBO records published are logged to the console with:
   - Sequential record number
   - Timestamp event
   - Price
   - Size
   - Order ID
   - Side (BID/ASK)
   - Action type
   - Sequence number
   - Symbol

## FastDDS Configuration

- **Domain ID**: 0 (default)
- **Topic Name**: MarketData
- **Transport Protocol**: FastDDS/RTPS over TCP
- **Discovery**: Simple Discovery Protocol (SDP)
- **Discovery Ports**: 7400-7410 (UDP)
- **RTPS Port**: 11511 (TCP)

## Signal Handling

The application gracefully handles:
- `SIGINT` (Ctrl+C): Initiates clean shutdown
- `SIGTERM`: Initiates clean shutdown

All resources are properly released before exit.

## Dependencies

- FastDDS (libfastrtps)
- FastCDR (libfastcdr)
- C++17 or later
- Linux/Unix system with socket support

## Troubleshooting

### Subscriber starts but receives no data
1. Ensure the publisher is running first
2. Check that both processes are on the same network interface
3. Verify FirewallRules (if using firewall)
4. Check if FastDDS discovery ports are not blocked

### Compilation errors
1. Ensure FastDDS development headers are installed: `libfastrtps-dev`
2. Verify the include path points to correct FastDDS headers
3. Check that FastCDR development files are available

## Notes

- The subscriber is designed to work with actual FastDDS publishers
- Console output includes all MBO record details for debugging and monitoring
- The application runs indefinitely until interrupted (Ctrl+C)
- No file logging is performed; all output is to console only
