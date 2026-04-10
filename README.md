#Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator#
**High-Fidelity Zonal Orchestration on QNX RTOS**

[![RTOS: QNX 7.1](https://img.shields.io/badge/RTOS-QNX_7.1-blue.svg)](https://blackberry.qnx.com/)
[![IPC: MsgSend/MsgReceive](https://img.shields.io/badge/IPC-QNX_Message_Passing-green.svg)]()
[![Core: 8-Core Scaling](https://img.shields.io/badge/CPU-8_Core_Affinity-orange.svg)]()

# Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator  
High-Fidelity Zonal Orchestration on QNX RTOS

---

## 1. Executive Summary

This project implements a deterministic ADAS sensor aggregation system on QNX RTOS with focus on real-time scheduling, IPC, and multi-core execution.

---

## Phase 1: v1.0 — Computational Stress Test

### CPU Activity
![CPU Activity](./OUTPUTS/version1/CPU Activity.png)

Shows how CPU behaves under heavy computation.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version1/CPU usage.png)

Stable usage under load.

---

### CPU Migration
![CPU Migration](./OUTPUTS/version1/CPU Migration.png)

Thread movement across cores due to no affinity.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version1/inter CPU communication.png)

Basic communication pattern.

---

### Summary
![Summary](./OUTPUTS/version1/summary.png)

Overall performance.

---

### Timeline
![Timeline](./OUTPUTS/version1/Timeline.png)

Execution consistency.

---

## Phase 2: v2.0 — Multicore Transition

### CPU Activity
![CPU Activity](./OUTPUTS/version2/CPU Activity.png)

Better core distribution.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version2/CPU Usage.png)

Improved balance.

---

### CPU Migration
![CPU Migration](./OUTPUTS/version2/CPU Migration.png)

Reduced migration.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version2/inter CPU communication.png)

Shared memory communication.

---

### Summary
![Summary](./OUTPUTS/version2/Summary.png)

Improved efficiency.

---

### Timeline
![Timeline](./OUTPUTS/version2/Timeline.png)

More predictable execution.

---

## Phase 3: v3.0 — IPC Integration

### CPU Activity
![CPU Activity](./OUTPUTS/version3/CPU Activity.png)

Stable scheduling.

---

### CPU Usage
![CPU Usage](./OUTPUTS/version3/CPU Usage.png)

Efficient utilization.

---

### Inter-CPU Communication
![Inter CPU Communication](./OUTPUTS/version3/inter CPU communication.png)

Deterministic message passing.

---

### Summary
![Summary](./OUTPUTS/version3/Summary.png)

Stable IPC performance.

---

### Timeline
![Timeline](./OUTPUTS/version3/Timeline.png)

Deterministic execution.

---

## Phase 4: v4.0 — Zonal Scaling

100 sensors distributed across zones:
- Vision
- Radar
- Proximity
- Vitals

Zonal processing reduces IPC load and improves scalability.

---

## Benchmark Analysis

### Summary
![Summary](./OUTPUTS/benchmark/Summary.png)

Overall performance.

---

### CPU Usage
![CPU Usage](./OUTPUTS/benchmark/CPU usage.png)

Consistent usage.

---

### CPU Activity
![CPU Activity](./OUTPUTS/benchmark/CPU activity.png)

Deterministic scheduling.

---

### Timeline
![Timeline](./OUTPUTS/benchmark/timeline.png)

Low jitter execution.

---

## Final Insight

> ADAS performance depends on deterministic execution, not just algorithms.

---

Q-eHack 2026 Submission
