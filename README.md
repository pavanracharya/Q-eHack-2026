#Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator#
**High-Fidelity Zonal Orchestration on QNX RTOS**

[![RTOS: QNX 7.1](https://img.shields.io/badge/RTOS-QNX_7.1-blue.svg)](https://blackberry.qnx.com/)
[![IPC: MsgSend/MsgReceive](https://img.shields.io/badge/IPC-QNX_Message_Passing-green.svg)]()
[![Core: 8-Core Scaling](https://img.shields.io/badge/CPU-8_Core_Affinity-orange.svg)]()

# Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator  
High-Fidelity Zonal Orchestration on QNX RTOS

---

## 1. Executive Summary

This project implements a deterministic ADAS sensor aggregation system on QNX RTOS.  
The focus is on real-time scheduling, inter-process communication (IPC), and multi-core execution under high sensor load.

---

## Phase 1: v1.0 — Computational Stress Test

### CPU Activity
![CPU Activity](./OUTPUTS/version1/CPU Activity.png)

This shows how CPU cores behave under heavy computation. The scheduler distributes workload efficiently.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version1/CPU usage.png)

Demonstrates stable CPU usage even under continuous load.

---

### CPU Migration
![CPU Migration](./OUTPUTS/version1/CPU Migration.png)

Threads migrate across cores due to lack of affinity, reducing determinism.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version1/inter CPU Communication.png)

Initial communication between threads without structured IPC.

---

### Summary
![Summary](./OUTPUTS/version1/summary.png)

Overall system performance overview.

---

### Timeline
![Timeline](./OUTPUTS/version1/Timeline.png)

Execution timing consistency under load.

---

## Phase 2: v2.0 — Multicore Transition

### CPU Activity
![CPU Activity](./OUTPUTS/version2/CPU Activity.png)

Improved workload distribution across multiple cores.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version2/CPU Usage.png)

Better CPU balancing compared to Phase 1.

---

### CPU Migration
![CPU Migration](./OUTPUTS/version2/CPU Migration.png)

Reduced migration due to affinity control.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version2/inter CPU Communication.png)

Threads now communicate using shared memory and synchronization.

---

### Summary
![Summary](./OUTPUTS/version2/Summary.png)

Improved performance and reduced contention.

---

### Timeline
![Timeline](./OUTPUTS/version2/Timeline.png)

More predictable execution.

---

## Phase 3: v3.0 — IPC Integration

### CPU Activity
![CPU Activity](./OUTPUTS/version3/CPU Activity.png)

Stable execution using message passing.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version3/CPU Usage.png)

Efficient CPU utilization.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version3/inter CPU communication.png)

Deterministic communication using QNX IPC.

---

### Summary
![Summary](./OUTPUTS/version3/Summary.png)

Stable IPC performance under load.

---

### Timeline
![Timeline](./OUTPUTS/version3/Timeline.png)

Highly predictable execution timing.

---

## Phase 4: v4.0 — Zonal Scaling

The system scales to simulate 100 sensors across 4 zones:

- Vision  
- Radar  
- Proximity  
- Vitals  

Each zone aggregates data before sending it to the fusion engine, reducing IPC load and improving efficiency.

---

## Benchmark Analysis

### Summary
![Summary](./OUTPUTS/Bench Mark/Summary.png)

Overall system performance comparison.

---

### CPU Usage
![CPU Usage](./OUTPUTS/Bench Mark/CPU usage.png)

Stable CPU usage under high load.

---

### CPU Activity
![CPU Activity](./OUTPUTS/Bench Mark/CPU activity.png)

Deterministic scheduling behavior.

---

### Timeline
![Timeline](./OUTPUTS/Bench Mark/timeline.png)

Low jitter and consistent execution.

---

## Final Insight

> ADAS performance depends not just on algorithms,  
> but on deterministic execution, timing, and synchronization.

QNX enables this through its microkernel architecture, priority scheduling, and message-driven communication.

---

Q-eHack 2026 Submission
