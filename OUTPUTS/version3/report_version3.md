# Detailed Output Report: Version 3 (v3_adas)
## Service-Oriented Architecture & Hybrid Zero-Copy IPC

This report provides the full technical breakdown for Version 3 of the QNX ADAS Sensor Aggregation project. This version marks the transition from a monolithic process to a **Decoupled Service Model** designed for high-bandwidth data fusion.

---

## 1. Architectural Objective
Version 3 introduces the concept of **Component Independence**. The key objectives were:
1.  **Shared Memory (Zero-Copy) Integration**: Moving high-bandwidth vision data (object lists, labels) out of the message buffer and into mapped memory to avoid CPU cycles spent on copying.
2.  **Service Decoupling**: Separation of Fusion Engine, Sensor Drivers, and Health Monitoring into independent logical units.
3.  **Dynamic Reliability Weighting**: Implementing a logic layer that penalizes "Stale Data" (jitter/latency) by reducing a sensor's contribution to final fusion.

---

## 2. Technical Specializations

### 2.1 Hybrid IPC Model
Version 3 optimizes data flow by using different IPC mechanisms for different data sizes:

| Mechanism | Usage | Rationale |
| :--- | :--- | :--- |
| **QNX Messages** | Metadata / Pulse Heartbeats | Reliable, synchronous notification. Low overhead for small payloads. |
| **Shared Memory (SHM)** | Vision Object Data | **Zero-Copy**. The sensor writes directly to a memory segment that the Fusion Engine maps, eliminating the `memcpy` overhead of standard messaging. |

### 2.2 Shared Memory Implementation (Vision)
The Vision service (`Vision_V3`) utilizes a sophisticated producer-consumer model via `/dev/shmem`:

1.  **Allocation**: The system creates a memory object via `shm_open(SHM_VISION_PATH)`.
2.  **Mapping**: Both the Vision Producer and Fusion Consumer use `mmap()` to point their local address space to the same physical hardware memory.
3.  **Synchronization**: The producer writes the frame -> then sends a tiny `adas_metadata_msg_t` via `MsgSend` to notify the consumer that the buffer is "Dirty" (ready for read).

### 2.3 Reliability & Stale Data Detection
The Fusion Engine (`Fusion_Eng_V3`) now performs a "Time Difference" check on every incoming message.

```c
now_ns = (uint64_t)time(NULL) * 1000000000ULL;
int64_t delay_ms = (now_ns - msg.timestamp_ns) / 1000000;

if (delay_ms > 100) {
    sensor_weights[msg.sensor_id] *= 0.9f; // Penalize stale sensor
}
```
*   **Threshold**: 100ms.
*   **Penalty**: 10% reduction in fusion weight per delayed frame.
*   **Result**: This ensures that even if a sensor is suffering from "Camera Lag" (common in Version 4 simulations), the overall system safety is maintained by trusting more deterministic sensors (like Radar).

---

## 3. Component Breakdown

| Component | Priority | Core | Logic |
| :--- | :--- | :--- | :--- |
| **Fusion Engine** | 22 | Core 1 | Central aggregator; manages sensor weights and executes fusion algorithms. |
| **Vision Proc** | 20 | Core 2 | High-bandwidth SHM producer; performs simulated object detection (Labels, BBoxes). |
| **Health Monitor** | 25 | Float | Watchdog service; listens for heartbeat pulses from all active zones. |

---

## 4. Analytical Observations (from Logs)

Logs from Version 3 demonstrate the "Intelligent Fusion" behavior:

```text
[VISION] Shm Mapped and Connected. Starting Heavy Processing...
[FUSION] Server Ready. CHID: 1

[FUSION] RECV Sensor 3 | Reliability: 0.50 | Action: Fusion Applied
[FUSION] WARNING: Sensor 3 data stale (112 ms delay). Reducing weight.
[FUSION] RECV Sensor 3 | Reliability: 0.45 | Action: Fusion Applied
```

###  Analytical Deep-Dive
1.  **Impact of Stale Data**: The "WARNING" log confirms the reliability engine is active. In this trace, Sensor 3 (Vision) was delayed by 112ms, triggering an immediate safety penalty.
2.  **Pulse Handling**: The Fusion Engine code includes logic to detect `rcvid == 0` (Pulses), allowing it to process heartbeats without being blocked by data-heavy sensor threads.
3.  **Core Efficiency**: By mapping Shared Memory, the CPU usage on Core 1 (Fusion) remains low even as the data size of vision frames increases—a significant improvement over Version 2.

---

## 5. Conclusion: Foundations for Scaling
Version 3 introduced the "Safety Logic" required for autonomous operations. By decoupling components and implementing hard reliability scores, the system can now survive partial sensor failure. This architecture was the direct precursor to **Version 4**, where this model was scaled to **100 concurrent sensors** across a hierarchy of Zonal Aggregators.

---

> [!NOTE]
> **Technical Trade-off**: While Shared Memory is faster, it requires careful synchronization to prevent "Race Conditions" (reading while the sensor is half-way through a write). Version 3 relies on the synchronous `MsgSend` notification to gate access, essentially using the message as a **Distributed Semaphore**.
