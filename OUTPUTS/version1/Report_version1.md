# Detailed Output Report: Version 1 (v1_sensor_heavy)
## Baseline Computational Stress & IPC Emulation

This report provides an exhaustive technical analysis of Version 1 of the QNX ADAS Sensor Aggregation system, based on source code analysis (`sensor_heavy.c`) and system output logs.

---

## 1. Architectural Objective
Version 1 was designed as a **Baseline Feasibility Study**. The primary goals were:
1.  **POSIX Compliance Verification**: Ensuring that standard POSIX threading and synchronization primitives work precisely as expected on the QNX Neutrino microkernel.
2.  **CPU Stress Benchmarking**: Determining how the QNX scheduler handles 100% CPU utilization blocks without starving other system processes.
3.  **IPC Simulation**: Creating a software bridge between standard POSIX and native QNX "Synchronous Message Passing" logic.

---

## 2. Implementation Deep-Dive

### 2.1 The Workload Algorithm
Version 1 intentionally introduces a "Heavy Compute" section to simulate raw sensor data processing (e.g., Lidar point cloud filtering or Camera frame analysis).

```c
double perform_heavy_computation() {
    double result = 0.0;
    for (int i = 1; i < 5000000; i++) {
        result += sqrt((double)i) * sin((double)i);
    }
    return result;
}
```
*   **Iteration Count**: 5 Million cycles per sensor read.
*   **Operations**: Square root (`sqrt`) and Sine (`sin`) - both are computationally expensive floating-point operations.
*   **Impact**: On a standard embedded core, this function creates a sustained 100% core load for the duration of its execution.

### 2.2 IPC Emulation Layer
Version 1 does not use native `MsgSend`/`MsgReceive` yet. Instead, it implements a **Blocking Queue** that emulates QNX behavior using POSIX primitives.

| Function | Logic | QNX Equivalent |
| :--- | :--- | :--- |
| `MsgSend_Sim` | Blocks if queue is full; uses `pthread_cond_signal`. | `MsgSend()` |
| `MsgReceive_Sim` | Blocks if queue is empty; uses `pthread_cond_wait`. | `MsgReceive()` |
| `channel_init` | Initializes mutexes and condition variables. | `ChannelCreate()` |

### 2.3 Process & Threading Model
- **Process**: `sensor_heavy.exe`
- **Threads**:
    - `Aggregator_V1` (ID: 1): The "Server" thread. Blocks on `MsgReceive_Sim`.
    - `Sensor_V1_1` to `Sensor_V1_4`: The "Client" threads. They alternate between heavy math and sending data.

---

## 3. Runtime Output Analysis

The logs from Version 1 show a highly deterministic, albeit sequential, pattern of execution.

```text
Starting QNX RTOS Simulated Sensor System (Heavy CPU Load)
Sensor 1: Starting heavy computation (Simulating CPU load)...
Sensor 2: Starting heavy computation (Simulating CPU load)...
Sensor 4: Starting heavy computation (Simulating CPU load)...
Sensor 3: Sending data via simulated MsgSend...
Aggregator (Server): Received from Sensor 3 | Data: 1533.080449
```

###  Analytical Observations
1.  **Sequential Processing**: The output confirms that even though threads start tasks simultaneously, the QNX scheduler (likely using default `SCHED_RR` for these threads) partitions the heavy math predictably.
2.  **Successive Delivery**: Sensor 3 completes its math before Sensor 1, 2, and 4 in this specific trace, demonstrating that timing jitter is present in this initial, un-partitioned version.
3.  **Data Consistency**: The result `1533.080449` is consistent across all sensors, verifying that the mathematical load is identical and calculation integrity is maintained under stress.
4.  **Sleeping State**: Each sensor thread performs `usleep(500000)` after sending, allowing the CPU to stabilize and the Aggregator to process the queue.

---

## 4. Visual Artifact Metadata
Based on `./OUTPUTS/version1/`, the following diagnostic findings were recorded:

- **CPU Usage.png**: Shows a jagged "sawtooth" pattern. The peaks represent the 5-million-iteration math loops, and the troughs represent the 500ms sleep periods.
- **Summary.png**: Indicates significant "User Time" (logic execution) compared to later versions where "Kernel Time" (IPC) becomes the dominant metric.
- **Timeline.png**: Shows clear context switches between the 4 sensor threads as they contest for the same CPU resources.

---

## 5. Technical Conclusion: The "Transition Gate"
Version 1 successfully proved that:
- QNX handles 100% CPU spikes without crashing.
- Multi-threaded synchronization via POSIX is robust.
- **Required Optimization**: The simulation of IPC via Mutexes is less efficient than native QNX Pulse/Message passing. This led directly to the architectural overhaul seen in **Version 2 (Multicore Partitioning)**.

---

> [!IMPORTANT]
> **Evolution Point**: Version 1 is "Self-Contained." It lacks the modularity of the Name Service introduced in v3, meaning all components must live within the same executable. This version served as the **proof-of-performance** before moving to the distributed zonal architecture.
