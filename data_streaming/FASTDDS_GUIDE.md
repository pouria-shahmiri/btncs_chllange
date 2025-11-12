# FastDDS Subscriber Connection & Latency Guide

## How FastDDS Subscriber Discovers & Connects to Publisher

### Overview
FastDDS uses an automatic discovery mechanism called **Simple Discovery Protocol (SDP)** to find publishers without requiring manual connection strings or IP addresses.

### Connection Flow

```
┌─────────────────────────────────────────────────────────────┐
│ 1. INITIALIZATION PHASE                                     │
├─────────────────────────────────────────────────────────────┤
│ Publisher:                  │ Subscriber:                   │
│ - Create DomainParticipant  │ - Create DomainParticipant   │
│   (Domain ID: 0)            │   (Domain ID: 0)             │
│ - Join discovery service    │ - Join discovery service      │
│ - Port: 7400-7410           │ - Port: 7400-7410            │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ 2. TOPIC ANNOUNCEMENT PHASE                                 │
├─────────────────────────────────────────────────────────────┤
│ Publisher:                  │ Subscriber:                   │
│ - Announce Topic:           │ - Subscribe to Topic:         │
│   "MarketData"              │   "MarketData"                │
│ - Broadcast via multicast   │ - Broadcast interest          │
│   or unicast discovery      │   (via discovery endpoints)   │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ 3. MATCHING PHASE                                           │
├─────────────────────────────────────────────────────────────┤
│ - Topic Names Match: "MarketData" == "MarketData" ✓         │
│ - Domain IDs Match: 0 == 0 ✓                                │
│ - QoS Policies Compatible: ✓                                │
│ - Built-in Participant Discovery (PDP) exchanges metadata   │
│ - Built-in Endpoint Discovery (EDP) finds matching endpoints│
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│ 4. DATA TRANSPORT CONNECTION                                │
├─────────────────────────────────────────────────────────────┤
│ - TCP connection established (port 11511)                   │
│ - RTPS handshake performed                                  │
│ - Data stream begins flowing from publisher → subscriber    │
│ - Latency: ~0.5-1.0 ms (for local TCP on same machine)      │
└─────────────────────────────────────────────────────────────┘
```

### Key Configuration Points

**Domain ID (DDS_DOMAIN_ID_ENV_VARIABLE)**
- Must be same for all communicating participants
- Default: 0
- Allows isolating multiple DDS systems on same network

**Topic Name**
- Must match exactly between publisher and subscriber
- In this project: "MarketData"
- Case-sensitive

**QoS Policies (Quality of Service)**
- Reliability: BEST_EFFORT or RELIABLE
- History: KEEP_ALL or KEEP_LAST(N)
- Durability: VOLATILE, TRANSIENT, or PERSISTENT
- Must be compatible between publisher and subscriber

### Network Ports Used

| Component | Port Range | Protocol | Purpose |
|-----------|-----------|----------|---------|
| RTPS Discovery | 7400-7410 | UDP | Participant discovery (multicast) |
| RTPS Data | 11511 | TCP | Actual data transport |
| Unicast Discovery | 7400+ | TCP | Fallback discovery (unicast) |

### Environment Variables

```bash
# Set verbosity (for debugging)
export DDS_VERBOSITY=4

# Set domain ID
export ROS_DOMAIN_ID=0

# Enable QoS logging
export FASTDDS_STATISTICS_STDOUT=0
```

### How to Verify Connection

```bash
# Check if subscriber found publisher
./market_data_subscriber
# Look for: "Listening for market data on topic 'MarketData'..."

# Monitor with netstat
netstat -an | grep 11511  # RTPS port
netstat -an | grep 7400   # Discovery port
```

---

## Latency Monitoring

### What is Latency in DDS?

**End-to-End (E2E) Latency** = Time from publisher sending data to subscriber receiving it

```
Publisher Timeline:
│ 0 ms   → 0.1 ms  → 0.2 ms  │ 0.3 ms
│ [Send] → [RTPS]  → [TCP]   │ [Rcv]
└─────────────────────────────┘
  Latency: ~0.3 ms
```

### Factors Affecting Latency

| Factor | Impact | Mitigation |
|--------|--------|-----------|
| Network Distance | High | Use local network/LAN |
| Message Size | Medium | Keep messages small (63 bytes) |
| QoS Settings | High | Use BEST_EFFORT + KEEP_LAST |
| CPU Load | Medium | Reduce other processes |
| Transport | Low | TCP typically ~0.5-1.0 ms |

### Typical Latency Values

| Scenario | Latency |
|----------|---------|
| Same machine (TCP) | 0.5-1.0 ms |
| Local network (1 Gbps) | 1-5 ms |
| 10Gbps network | 0.1-1 ms |
| WAN/Internet | 10-100+ ms |

### Monitoring in Application

Our subscriber logs latency metrics:

```cpp
// Every 100 messages
📊 Latency Statistics (Last 100 messages):
   Average Latency: ~0.5-1.0 ms (typical for local TCP)
   Min Latency:     ~0.1 ms
   Max Latency:     ~5.0 ms

// Every 200 messages
🔍 Latency Analysis:
   - Message ID:       1234
   - Publish Time:     1234567890 ns
   - Receive Time:     1234567898 ns
   - E2E Latency:      8 ns = 0.008 μs
```

---

## Advanced: How Discovery Works in Detail

### 1. Multicast Discovery (Default)

```
Step 1: Publisher sends multicast (224.0.0.0-239.255.255.255)
        ↓
Step 2: Subscriber receives multicast on same network segment
        ↓
Step 3: Subscriber responds via unicast (direct to publisher)
        ↓
Step 4: Publisher and Subscriber exchange metadata
        ↓
Step 5: Connection established
```

### 2. Unicast Discovery (Fallback)

Used when multicast not available or disabled:
```
Step 1: Subscriber looks up "locator_list" (predefined endpoints)
        ↓
Step 2: Sends discovery messages to specific IP:port
        ↓
Step 3: Publisher responds if found
        ↓
Step 4: Metadata exchange and connection
```

### 3. QoS Matching

FastDDS checks QoS compatibility:
```
Publisher QoS:              Subscriber QoS:
- Reliability: RELIABLE     - Reliability: RELIABLE  ✓ MATCH
- History: KEEP_LAST(10)    - History: KEEP_LAST(N)  ✓ COMPATIBLE
- Durability: VOLATILE      - Durability: VOLATILE   ✓ MATCH
```

---

## Troubleshooting Subscriber Connection Issues

### Issue: Subscriber not finding Publisher

**Possible Causes:**
1. Different Domain IDs
2. Different Topic Names
3. Firewall blocking ports 7400-7410, 11511
4. Multicast disabled on network

**Solutions:**
```bash
# Check Domain ID
echo $ROS_DOMAIN_ID

# Check if ports are listening
netstat -an | grep LISTEN

# Enable debug output
export DDS_VERBOSITY=4
./market_data_subscriber

# Check firewall
sudo ufw allow 7400:7410/udp
sudo ufw allow 11511/tcp
```

### Issue: High Latency

**Possible Causes:**
1. Network congestion
2. Large message size
3. High CPU load
4. QoS settings too strict

**Solutions:**
```bash
# Reduce message size (already 63 bytes - optimal)
# Use BEST_EFFORT instead of RELIABLE
# Monitor CPU: top -u username
# Check network: iperf3
```

### Issue: Subscriber crashes on startup

**Possible Causes:**
1. Memory issues
2. Missing FastDDS libraries
3. Incompatible QoS

**Solutions:**
```bash
# Verify FastDDS installed
ldconfig -p | grep fastdds

# Increase stack size if needed
ulimit -s unlimited

# Run with GDB for detailed error
gdb ./market_data_subscriber
```

---

## FastDDS vs Other Middleware

| Feature | FastDDS | ROS | Apache Kafka |
|---------|---------|-----|--------------|
| Discovery | Automatic | Via rosmaster | Manual | 
| Latency | ~0.5-1 ms | ~5-10 ms | ~5-50 ms |
| Overhead | Low | Medium | High |
| Scalability | Excellent | Good | Excellent |
| Network | Local/LAN | Any | Any |

---

## Performance Optimization Tips

### Publisher Side
- Batch messages when possible
- Use shared memory transport for same-machine
- Set QoS to BEST_EFFORT + KEEP_LAST(1)

### Subscriber Side
- Use thread pool for message processing
- Avoid blocking operations in listener
- Buffer incoming messages in queue

### Network
- Use dedicated NIC for DDS traffic
- Enable jumbo frames (MTU 9000)
- Disable TCP Nagle's algorithm (TCP_NODELAY)

---

## Real-World Example Output

```
=== FastDDS Subscriber Configuration ===
FastDDS Subscriber initialized (TCP mode)
Listening for market data on topic 'MarketData'...

📡 FastDDS Connection Details:
   Domain ID:           0 (default)
   Topic Name:          MarketData
   Transport Protocol:  TCP/IP
   Default Port:        11511 (RTPS), 7400-7410 (Discovery)

🔍 FastDDS Discovery Mechanism:
   1. Simple Discovery Protocol (SDP)
   2. Subscriber joins domain 0 and announces interest in 'MarketData' topic
   3. Built-in discovery endpoints exchange metadata
   4. When publisher publishes on same topic+domain, subscriber matches
   5. TCP connection established automatically between matched endpoints

📋 How Subscriber Connects to Publisher:
   • Both use same Domain ID (0)
   • Both use same Topic Name ('MarketData')
   • QoS policies must be compatible (reliability, durability, etc.)
   • Automatic handshake via RTPS protocol
   • TCP connection established for data transport
   • No manual connection string needed!

✅ Subscriber ready to receive messages
```

---

## Further Reading

- FastDDS Official Documentation: https://fast-dds.docs.eprosima.com/
- RTPS Protocol: https://www.omg.org/spec/RTPS/2.2
- DDS Standard: https://www.omg.org/spec/DDS/
