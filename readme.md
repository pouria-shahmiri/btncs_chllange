# Low-Latency Trading Infra: My Approach to the Challenge

This has been a super interesting challenge for me! It brought back a ton of memories from my time running a market-making startup with my quant friend (we raised almost \$1 million, but, well, we failed after 3 years :DDD ).

During those startup days, I architected and built our entire low-latency infrastructure. I tried practically everything to squeeze out every millisecond for fast execution and calculating . I chose Data Streaming and Order Book Reconstruction as the focus for this exam because, honestly, they are the two main pillars of any successful low-latency setup:

1.  **How you pass data** from Service A to Service B.
2.  **How you reconstruct the order book** to achieve that low-low-low latency.

The main game here is smart technology selection and robust implementation.

***

## My Infra Philosophy: Quantizing Risk

As a quant dev, I think about risk all the time, but it's not just market and strategy risk—we have to quantize and calculate the risk at the infra level too.

My key considerations were:

| Principle | Why It Matters |
| :--- | :--- |
| **Risk of Data Transfer** | You can't have a reliable strategy if your market data is flaky or delayed due to a bad pipe. |
| **Speed & Reliability** | The order book must be reconstructed very fast and reliably. Performance is non-negotiable. |
| **Don't Build from Scratch** | Reinventing the wheel adds unnecessary risk (bugs, long dev cycles). We must use proven, reliable technologies. |

***

##  Data Streaming: Service A -> Service B

I've been through the whole technology journey here:

* **TCP/Sockets:** Fast, but we ran into annoying overheads.
* **gRPC:** Good for structure, but sometimes unreliable or too heavy for true sub-millisecond needs.
* **Kafka:** Reliable as hell, but it's a massive operational risk. You have to maintain it, and it's not truly low-latency compared to what a top-tier firm needs.

So, we needed a sweet spot: something fast like TCP/gRPC, and highly reliable/distributed like Kafka, but without the operational burden of a broker.

That’s why I picked **FastDDS** (and also considered **Chronicle Queue**).

### FastDDS: Brokerless Pub/Sub

FastDDS is phenomenal. It's built on the Data Distribution Service (DDS) standard, which is perfect for mission-critical, low-latency, and high-reliability communications. It feels like Kafka as a broker, but it’s distributed and brokerless. It’s also as fast as a tuned TCP connection but without the typical overhead.

| Pros of FastDDS | Cons of FastDDS |
| :--- | :--- |
| **Brokerless:** Zero central point of failure, minimal operational risk. | **Configuration Complexity:** QoS settings can be intricate to master. |
| **Ultra-Low Latency:** Optimized for speed, often using multicast. | **Network Dependency:** Requires a high-quality, stable network (especially if using multicast) to maximize benefits. |
| **High Reliability (QoS):** Built-in Quality of Service for guaranteed or best-effort delivery. | **Learning Curve:** DDS is a different paradigm than traditional message queues. |

*I also love Chronicle Queue for pure, high-throughput Inter-Process Communication (IPC) on a single machine, which would be the fastest path, but FastDDS offers the flexibility for distributed services.*

***

## Order Book Reconstruction: The Engine

To mirror the order book accurately, you need a high-performance Matching Engine. Building that from scratch is a massive, unforgivable risk. It's the ultimate example of "don't build from scratch."

I found and settled on Liquibook. We used it in the startup and it was a great.

### Liquibook: The Proven Matching Engine

This C++ library can handle 2 to 2.5 million records per second easily. The real scalability trick here is in the deployment: using a platform like Kubernetes, you can deploy an independent, dedicated Liquibook service for *each instrument/symbol*. This means if you have 100 symbols, your infra can theoretically handle 200+ million records per second total—that's true low-latency scalability.

By combining the low-risk communication of FastDDS and the proven logic of Liquibook, we are not building any wheels and we have a very fast, risk-averse data pipe and order book mirror.

| Pros of Liquibook | Cons of Liquibook |
| :--- | :--- |
| **Battle-Tested Logic:** Handles complex order management logic (Price-Time Priority, hidden orders, etc.) reliably. | **External Dependency:** Reliance on an external library for core logic. |
| **Extreme Performance:** Built in C++ for maximum throughput (2M+ updates/sec per instance). | **C++ Integration:** Requires robust C++ development practices for successful integration. |
| **MBO Support:** Designed to handle **Message-By-Order (MBO)** data needed for this challenge. | |

***

## 🔬 Testing and My Mindset

My testing mindset goes beyond simple unit tests. For a system like this, the two things that matter most are **Correctness** and **Latency**.

1.  **Correctness & Reliability:**
    * **Book Snapshot Validation:** The most critical test is verifying that the reconstructed order book, at any given timestamp, is a **perfect, exact mirror** of the official source data (as mentioned in the challenge deliverables). This is the QA gold standard.
    * **Resilience Testing (Infra Risk):** We must simulate failures—killing the Order Book service (pod), dropping the network connection, and verifying that the FastDDS subscriber can reconnect and reliably sync or receive all buffered messages (using the appropriate QoS profile). The system must demonstrate graceful failure handling.

2.  **Performance & Latency:**
    * **Throughput Testing:** Measure how many messages/second the system can process while maintaining the required latency profile (e.g., hitting the **500K msg/sec target**).
    * **Tick-to-Book Latency:** Crucially, we need to measure the **end-to-end latency**—from the moment a message leaves the `Data Streaming` service to the moment the `Order Book Reconstruction` service has processed the update and is ready to trade. We must monitor the **p99 latency** (aiming for sub-10ms, or even sub-1ms where possible) to ensure consistency.

The goal isn't just a working solution; it's a **production-ready, ultra-fast, and risk-managed infrastructure component.** That's the real challenge.


also my approach to use AI : i hate vibecoding and i get the whole context of the project with directories and files relation and i give that to AI and get what i want as a change.