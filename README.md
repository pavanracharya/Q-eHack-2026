# Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator
## High-Fidelity Zonal Orchestration on QNX RTOS

[![RTOS: QNX 7.1](https://img.shields.io/badge/RTOS-QNX_7.1-blue.svg)](https://blackberry.qnx.com/)
[![IPC: MsgSend/MsgReceive](https://img.shields.io/badge/IPC-QNX_Message_Passing-green.svg)]()
[![Core: 8-Core Scaling](https://img.shields.io/badge/CPU-8_Core_Affinity-orange.svg)]()

---

## 1. Executive Summary
This project implements a deterministic ADAS sensor aggregation system on the QNX 7.1 Microkernel. By leveraging a **Zonal Architecture** and **8-Core Thread Affinity**, we manage high-bandwidth data streams from **100 concurrent sensors** with nanosecond-level determinism. This report documents the system's evolution from a single-core stress test to a full-scale zonal fusion platform.

---

## 2. System Evolution Lifecycle

### Phase 1: v1.0 — Computational Stress Test
Focus: Establishing baseline scheduling performance and context-switching stability.

| CPU Activity | CPU Usage | CPU Migration |
| :---: | :---: | :---: |
| ![CPU Activity](./OUTPUTS/version%201/CPU%20Activity.png) | ![CPU Usage](./OUTPUTS/version%201/CPU%20usage.png) | ![CPU Migration](./OUTPUTS/version%201/CPU%20Migration.png) |
| *Distribution under heavy load* | *Stable continuous utilization* | *Migration due to lack of affinity* |

| Inter-CPU Communication | Summary | Timeline |
| :---: | :---: | :---: |
| ![Inter-CPU](./OUTPUTS/version%201/inter%20CPU%20Communication.png) | ![Summary](./OUTPUTS/version%201/summary.png) | ![Timeline](./OUTPUTS/version%201/Timeline.png) |
| *Initial thread communication* | *Performance overview* | *Timing consistency* |

---

### Phase 2: v2.0 — Multicore Transition
Focus: Moving from centralized to partitioned execution with affinity control.

| CPU Activity | CPU Usage | CPU Migration |
| :---: | :---: | :---: |
| ![CPU Activity](./OUTPUTS/version%202/CPU%20Activity.png) | ![CPU Usage](./OUTPUTS/version%202/CPU%20Usage.png) | ![CPU Migration](./OUTPUTS/version%202/CPU%20Migration.png) |
| *Improved core balance* | *Optimized thread placement* | *Reduced migration jitter* |

| Inter-CPU Communication | Summary | Timeline |
| :---: | :---: | :---: |
| ![Inter-CPU](./OUTPUTS/version%202/inter%20CPU%20Communication.png) | ![Summary](./OUTPUTS/version%202/Summary.png) | ![Timeline](./OUTPUTS/version%202/Timeline.png) |
| *Shared memory synchronization* | *Reduced resource contention* | *Predictable execution flow* |

---

### Phase 3: v3.0 — Systemic IPC Integration
Focus: Decoupling drivers from fusion logic using QNX synchronous message passing.

| CPU Activity | CPU Usage | Inter-CPU Communication |
| :---: | :---: | :---: |
| ![CPU Activity](./OUTPUTS/version%203/CPU%20Activity.png) | ![CPU Usage](./OUTPUTS/version%203/CPU%20Usage.png) | ![Inter-CPU](./OUTPUTS/version%203/inter%20CPU%20communication.png) |
| *MsgSend/MsgReceive loops* | *Low-overhead IPC* | *Deterministic service discovery* |

| Summary | Timeline |
| :---: | :---: |
| ![Summary](./OUTPUTS/version%203/Summary.png) | ![Timeline](./OUTPUTS/version%203/Timeline.png) |
| *Scalable communication* | *Strict timing guarantees* |

---

### Phase 4: v4.0 — Zonal Scaling (Current)
The final architecture scales to simulate **100 Sensors** categorized into 4 hardware-abstracted zones.

**Zonal Configuration Matrix:**
| Zone | Priority | Core Cluster | Sensor Count | Primary Function |
| :--- | :--- | :--- | :--- | :--- |
| **Vision** | 20 | Cores 0-1 | 12 | Surround/Night/Thermal |
| **Radar** | 20 | Cores 1-2 | 20 | Long-range & Corner Sensing |
| **Proximity** | 20 | Cores 2-3 | 24 | Lidar & Ultrasonic Safety |
| **Vitals** | 20 | Core 3 | 44 | IMU, Wheel Speeds, BMS |

---

## 3. Technical Architecture

### 3.1 8-Core Thread Orchestration
The system leverages hardware partitioning to ensure safety-critical tasks are never starved of cycles.

```mermaid
graph TD
    S1[100 Sensors] -->|MsgSend| Z1[Vision Zone]
    S1 -->|MsgSend| Z2[Radar Zone]
    S1 -->|MsgSend| Z3[Proximity Zone]
    S1 -->|MsgSend| Z4[Vitals Zone]
    
    Z1 -->|Reduced Summary| CF[Central Fusion Engine]
    Z2 -->|Reduced Summary| CF
    Z3 -->|Reduced Summary| CF
    Z4 -->|Reduced Summary| CF
    
    CF -->|Decision| DL[Decision Logic]
    
    subgraph Core Affinity Cluster
        Z1-Z4 -.->|Mask: 0x0F| C03[Cores 0-3]
        CF -.->|Mask: 0xF0| C47[Cores 4-7]
    end
```

---

## 4. Benchmark Analysis

We benchmarked the system under peak load (100 sensors at 1kHz) to validate deterministic response times.

| Bench Summary | CPU Usage | CPU Activity | Timeline |
| :---: | :---: | :---: | :---: |
| ![Summary](./OUTPUTS/Bench%20Mark/Summary.png) | ![CPU Usage](./OUTPUTS/Bench%20Mark/CPU%20usage.png) | ![CPU Activity](./OUTPUTS/Bench%20Mark/CPU%20activity.png) | ![Timeline](./OUTPUTS/Bench%20Mark/timeline.png) |
| *Cumulative comparison* | *Deterministic headroom* | *Locked affinity behavior* | *Nanosecond jitter control* |

---

## 5. Fault Tolerance & Safety

The system implements a **Pulse-based Watchdog** pattern for failure detection. If a zone fails significantly, the Central Fusion engine triggers an immediate failover.

> [!IMPORTANT]
> **Safety Guarantee**: QNX's `SCHED_FIFO` scheduler ensures that high-priority radar interrupts (Priority 22) always preempt background simulation tasks (Priority 15).

---

## 6. Build and Deployment

```bash
# Compile for QNX Neutrino Target
make -C src/v4_scaling all

# Run the 8-Core Orchestrator
./src/v4_scaling/system/orchestrator_8core
```

---

## 💡 Final Insight
> "ADAS performance depends not just on algorithms, but on deterministic execution, timing, and synchronization."

QNX enables this through its microkernel isolation, priority inheritance, and message-driven communication.

---
**Q-eHack 2026 Submission**
*Innovation | Performance | Safety*
