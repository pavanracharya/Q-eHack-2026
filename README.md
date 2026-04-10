#Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator#
**High-Fidelity Zonal Orchestration on QNX RTOS**

[![RTOS: QNX 7.1](https://img.shields.io/badge/RTOS-QNX_7.1-blue.svg)](https://blackberry.qnx.com/)
[![IPC: MsgSend/MsgReceive](https://img.shields.io/badge/IPC-QNX_Message_Passing-green.svg)]()
[![Core: 8-Core Scaling](https://img.shields.io/badge/CPU-8_Core_Affinity-orange.svg)]()

# Q-eHack 2026: Deterministic 100-Sensor ADAS Aggregator  
**High-Fidelity Zonal Orchestration on QNX RTOS**

---

## 1. Executive Summary

This project presents a deterministic, multi-core ADAS sensor aggregation system built on the QNX Neutrino RTOS.  
The goal is to simulate and validate how real-world automotive systems handle **high-frequency, multi-sensor data streams** under strict real-time constraints.

Unlike basic simulations, this system focuses on:
- Deterministic scheduling
- Inter-process communication (IPC)
- CPU affinity control
- Real-time responsiveness under heavy load

---

## 2. System Evolution

The system was developed in multiple phases, each solving a deeper RTOS-level challenge.

---

## Phase 1: v1.0 — Computational Stress Test

### Objective
To test how QNX handles heavy CPU workloads and scheduling under pressure.

### Implementation
- Single process
- High CPU load (math-heavy operations)
- Timing validation using `nanosleep()` and `TimerTimeout()`

### Observations

#### CPU Activity
![](OUTPUTS/version%201/CPU%20Activity.png)

This graph shows how CPU cores behave under continuous heavy computation.  
You can observe how QNX distributes execution without starving tasks.

---

#### CPU Usage
![](OUTPUTS/version%201/CPU%20usage.png)

The CPU usage remains stable, indicating that QNX prevents overload spikes even under stress.

---

#### CPU Migration
![](OUTPUTS/version%201/CPU%20Migration.png)

Thread migration across cores is visible here.  
This confirms that without affinity control, threads move between cores, which may impact determinism.

---

#### Inter-CPU Communication
![](OUTPUTS/version%201/inter%20CPU%20communication.png)

This reflects how threads interact across cores.  
At this stage, communication is minimal and unstructured.

---

#### Summary Output
![](OUTPUTS/version%201/summary.png)

This consolidates performance metrics including scheduling behavior and execution consistency.

---

#### Execution Timeline
![](OUTPUTS/version%201/Timeline.png)

The timeline verifies that execution intervals remain consistent, proving basic RTOS stability.

---

## Phase 2: v2.0 — Multicore Transition

### Objective
To move from a single-threaded model to a multi-core, parallel execution system.

### Implementation
- POSIX threads
- CPU affinity using `ThreadCtl()`
- Shared memory + mutex synchronization

---

#### CPU Activity
![](OUTPUTS/version%202/CPU%20Activity.png)

Workload is now distributed across cores more effectively.

---

#### CPU Usage
![](OUTPUTS/version%202/CPU%20Usage.png)

Improved load balancing compared to Phase 1.

---

#### CPU Migration
![](OUTPUTS/version%202/CPU%20Migration.png)

Migration reduces due to affinity control, improving determinism.

---

#### Inter-CPU Communication
![](OUTPUTS/version%202/inter%20CPU%20communication.png)

Threads now communicate via shared memory, introducing synchronization complexity.

---

#### Summary
![](OUTPUTS/version%202/Summary.png)

Shows improved parallel efficiency and reduced contention.

---

#### Timeline
![](OUTPUTS/version%202/Timeline.png)

Execution becomes more structured and predictable.

---

## Phase 3: v3.0 — IPC Integration (QNX Core Strength)

### Objective
To implement true QNX-style communication using message passing.

### Implementation
- `MsgSend`, `MsgReceive`, `MsgReply`
- Name service (`name_attach`, `name_open`)
- Decoupled sensor + fusion architecture

---

#### CPU Activity
![](OUTPUTS/version%203/CPU%20Activity.png)

More stable distribution due to controlled IPC.

---

#### CPU Usage
![](OUTPUTS/version%203/CPU%20Usage.png)

Efficient utilization with reduced contention.

---

#### Inter-CPU Communication
![](OUTPUTS/version%203/inter%20CPU%20communication.png)

This is the key improvement:
- Structured communication
- Deterministic message passing

---

#### Summary
![](OUTPUTS/version%203/Summary.png)

Indicates stable IPC performance under load.

---

#### Timeline
![](OUTPUTS/version%203/Timeline.png)

Execution becomes highly predictable — a core requirement for ADAS.

---

## Phase 4: v4.0 — Zonal Scaling (Current System)

### Objective
To simulate a real ADAS system with large-scale sensor integration.

### Features
- 100 simulated sensors
- 4 zonal aggregators:
  - Vision
  - Radar
  - Proximity
  - Vitals
- 8-core CPU affinity mapping
- Central fusion engine

### Key Concept

Instead of sending all sensor data directly:
→ Sensors send data to zones  
→ Zones preprocess  
→ Fusion engine receives optimized data  

This reduces:
- IPC overhead
- CPU contention
- Latency

---

## 3. Benchmark Analysis (QNX vs General OS)

### Objective
To compare deterministic behavior under high load.

---

#### System Summary
![](OUTPUTS/Bench%20Mark/Summary.png)

Shows overall system efficiency and stability.

---

#### CPU Usage
![](OUTPUTS/Bench%20Mark/CPU%20usage.png)

QNX maintains consistent CPU usage even under high-frequency inputs.

---

#### CPU Activity
![](OUTPUTS/Bench%20Mark/CPU%20activity.png)

Highlights deterministic scheduling vs random spikes in non-RTOS systems.

---

#### Execution Timeline
![](OUTPUTS/Bench%20Mark/timeline.png)

Critical observation:
- Minimal jitter
- Predictable execution intervals

---

## 4. Key Technical Insights

### Deterministic IPC
QNX message passing ensures:
- No race conditions
- Predictable execution order

---

### CPU Affinity Control
Using runmasks:
- Prevents unnecessary migration
- Ensures critical threads always get CPU time

---

### Real-Time Scheduling
Using `SCHED_FIFO`:
- High-priority tasks (Radar, Safety) execute immediately
- Lower tasks (Camera) do not block critical operations

---

### Fault Tolerance
- Pulse-based watchdog detects failures
- Missing sensor data triggers fallback logic

---

## 5. Directory Structure
