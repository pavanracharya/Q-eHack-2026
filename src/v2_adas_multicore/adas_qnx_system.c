#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <string.h>

// ---------------------------------------------------------
// QNX NATIVE HEADERS
// Note: These will not compile on Windows natively.
// You must compile this file in the QNX Momentics IDE.
// ---------------------------------------------------------
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>

// ---------------------------------------------------------
// IPC MESSAGE STRUCTURES
// ---------------------------------------------------------

// Enum to clearly identify which sensor is sending data
typedef enum {
    SENSOR_LIDAR = 1,
    SENSOR_RADAR,
    SENSOR_CAMERA,
    SENSOR_ULTRASONIC
} SensorType_e;

// Define a custom message type identifier for our application
#define ADAS_MSG_TYPE 0x100

// Standard QNX IPC Message Format
typedef struct {
    uint16_t type;         // Determines the message struct type (ADAS_MSG_TYPE)
    SensorType_e sensor_id;
    double timestamp;      // A theoretical timestamp for Sensor Fusion algorithms
    double processed_data; // Result of the heavy task computation
} adas_msg_t;

// Standard QNX IPC Reply Format (Server -> Client)
typedef struct {
    int status;            // 0 = Success, -1 = Failure
} adas_reply_t;


// ---------------------------------------------------------
// GLOBAL SYSTEM STATE
// ---------------------------------------------------------
int fusion_server_chid; // Global Channel ID for the Sensor Fusion Server
int system_run = 1;     // Global execution flag

// ---------------------------------------------------------
// UTILS: HARD COMPUTATIONS (To simulate CPU Load)
// ---------------------------------------------------------
double perform_heavy_computation(int iterations) {
    double result = 0.0;
    for (int i = 1; i < iterations; i++) {
        result += sqrt((double)i) * sin((double)i);
    }
    return result;
}

// ---------------------------------------------------------
// UTILS: TIMER PULSE CONFIGURATION
// ---------------------------------------------------------
// This creates a POSIX timer that delivers a QNX Pulse deterministically 
// instead of relying on usleep() inside a loop.
timer_t create_pulse_timer(int coid, int pulse_code, int nsec_period) {
    struct sigevent event;
    timer_t timer_id;
    struct itimerspec itime;

    // Define the event to be a pulse delivered to the server connection
    SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, pulse_code, 0);

    // Create the POSIX timer
    if (timer_create(CLOCK_MONOTONIC, &event, &timer_id) == -1) {
        perror("timer_create failed");
        exit(EXIT_FAILURE);
    }

    // Set the timer parameters (periodic firing)
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = nsec_period; // First fire time
    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = nsec_period; // Periodic reload time
    
    timer_settime(timer_id, 0, &itime, NULL);
    
    return timer_id;
}


// ---------------------------------------------------------
// SENSOR THREADS (CLIENTS)
// ---------------------------------------------------------

// Generic Sensor Logic encapsulating the QNX Client behaviors
void* generic_sensor_client(void* arg) {
    SensorType_e type = *(SensorType_e*)arg;
    
    // 1. Establish connection to the Sensor Fusion Server Channel
    // ConnectAttach blocks until the server allows connection.
    int coid = ConnectAttach(ND_LOCAL_NODE, 0, fusion_server_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        perror("ConnectAttach failed");
        return NULL;
    }

    printf("Sensor %d connected to Fusion Engine.\n", type);

    int iterations = 0;
    int sleep_usec = 0;

    switch (type) {
        case SENSOR_LIDAR: 
            iterations = 5000000; // Heavy math load
            sleep_usec = 200000;  // High frequency (200ms)
            break; 
        case SENSOR_RADAR: 
            iterations = 100000;  // Medium math load
            sleep_usec = 100000;  // Extremely high frequency (100ms)
            break; 
        case SENSOR_CAMERA:
            iterations = 15000000; // Extremely Heavy Load (Simulating Image Processing)
            sleep_usec = 500000;   // Medium Frequency (500ms -> 2fps simul)
            break;
        case SENSOR_ULTRASONIC:
            iterations = 5000;    // Low Load
            sleep_usec = 1000000; // Low frequency (1s)
            break;
    }

    // Allocate memory for our message and reply buffers
    adas_msg_t msg;
    adas_reply_t reply;
    
    msg.type = ADAS_MSG_TYPE;
    msg.sensor_id = type;

    while (system_run) {
        // 2. Perform the actual work (simulating physical sensor data capture/processing)
        msg.processed_data = perform_heavy_computation(iterations);
        
        // Use standard time as theoretical timestamp
        msg.timestamp = (double)time(NULL);

        // 3. Send the data to the Server
        // MsgSend blocks the thread until the server receives the message AND calls MsgReply
        // This is perfectly synchronous IPC.
        if (MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1) {
            perror("MsgSend failed");
            break;
        }

        if (reply.status != 0) {
            printf("Sensor %d: Server rejected the data.\n", type);
        }

        // QNX Idiom: Using basic usleep for now on clients, 
        // though advanced QNX designs would use Pulses even for client work loops. 
        usleep(sleep_usec); 
    }

    ConnectDetach(coid);
    return NULL;
}


// ---------------------------------------------------------
// SENSOR FUSION ENGINE (SERVER)
// ---------------------------------------------------------
void* fusion_server_thread(void* arg) {
    int rcvid;
    adas_msg_t msg;
    adas_reply_t reply;

    // 1. Create a Channel for IPC
    fusion_server_chid = ChannelCreate(0);
    if (fusion_server_chid == -1) {
        perror("ChannelCreate failed");
        return NULL;
    }

    printf("Fusion Server running [CHID %d]. Waiting for messages...\n", fusion_server_chid);

    while (system_run) {
        // 2. Block until a message or pulse arrives.
        // During execution, if a high-priority sender (like Radar) sends a message,
        // QNX priority inheritance temporarily elevates this thread to match the sender's priority.
        rcvid = MsgReceive(fusion_server_chid, &msg, sizeof(msg), NULL);

        if (rcvid == -1) {
            perror("MsgReceive error");
            break;
        }

        // 3. Evaluate if it's a Pulse or a standard IPC Message
        if (rcvid == 0) {
            // It's a system Pulse. In standard QNX, usually we'd structure our pulses here.
            // Example: struct _pulse pulse; MsgReceive(chid, &pulse, sizeof(pulse), NULL);
            printf("Fusion Server received a Pulse. (Timer interruption?)\n");
            continue;
        }

        // 4. Handle Standard IPC Message
        if (msg.type == ADAS_MSG_TYPE) {
            printf("\n[FUSION ENGINE] RECV -> Sensor ID: %d | Data: %.4f | Priority Inheritance Handled\n", 
                   msg.sensor_id, msg.processed_data);

            // Do critical fusion math here...

            // 5. Reply to unblock the client thread
            reply.status = 0; // Success
            MsgReply(rcvid, 0, &reply, sizeof(reply));
        } else {
            // Unknown message type, unblock the client with an error
            printf("[FUSION ENGINE] ERR -> Unknown Message Type\n");
            reply.status = -1;
            MsgError(rcvid, ENOSYS); 
        }
    }

    ChannelDestroy(fusion_server_chid);
    return NULL;
}


// ---------------------------------------------------------
// MAIN INITIALIZATION
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    printf("Starting QNX ADAS Sensor Aggregation System\n\n");

    pthread_t server_th;
    pthread_t lidar_th, radar_th, camera_th, ultrasonic_th;

    struct sched_param param;
    pthread_attr_t attr;

    // We must initialize the thread attributes
    pthread_attr_init(&attr);
    // Tell the system we are using explicit scheduling policies (not inheriting from parent)
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // 1. Create Server Thread (Fusion Engine)
    // Server runs at a very high baseline priority, but dynamically matches its clients via IPC.
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 25; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&server_th, &attr, fusion_server_thread, NULL);

    // Give server time to create the channel before clients attempt ConnectAttach
    usleep(100000); 

    // --- SENSOR THREADS SPAWNING ---

    // Define sensor arguments to pass ID dynamically but predictably
    static SensorType_e id_lidar = SENSOR_LIDAR;
    static SensorType_e id_radar = SENSOR_RADAR;
    static SensorType_e id_camera = SENSOR_CAMERA;
    static SensorType_e id_ultra = SENSOR_ULTRASONIC;


    // 2. Launch LiDAR (SCHED_FIFO, Highest Priority Client = 24)
    param.sched_priority = 24; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&lidar_th, &attr, generic_sensor_client, &id_lidar);
    // Pin LiDAR to CPU Core 1 (Assuming 0-indexed cores, mask = 0x02)
    // ThreadCtl is a QNX exclusive call to manage processor affinity.
    int mask_lidar = 0x02; 
    ThreadCtl_r(_NTO_TCTL_RUNMASK, (void*)mask_lidar);


    // 3. Launch Radar (SCHED_FIFO, Highest Priority Client = 24)
    // Radar needs extreme speed, so same priority.
    param.sched_priority = 24; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&radar_th, &attr, generic_sensor_client, &id_radar);
    // Pin Radar to CPU Core 2
    int mask_radar = 0x04; 
    ThreadCtl_r(_NTO_TCTL_RUNMASK, (void*)mask_radar);


    // 4. Launch Camera (SCHED_RR - Time Sliced, Medium Priority = 20)
    // Camera is heavily computational. Time-slicing (Round-Robin) is beneficial 
    // so it doesn't starve the system if it locks a core.
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    param.sched_priority = 20; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&camera_th, &attr, generic_sensor_client, &id_camera);
    // Pin Camera to CPU Core 3
    int mask_camera = 0x08; 
    ThreadCtl_r(_NTO_TCTL_RUNMASK, (void*)mask_camera);


    // 5. Launch Ultrasonic (SCHED_OTHER (Standard), Low Priority = 10)
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    param.sched_priority = 10; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&ultrasonic_th, &attr, generic_sensor_client, &id_ultra);
    // Do not set CPU affinity - let the OS float this simple thread on any free core.


    // We join here and wait infinitely. 
    pthread_join(server_th, NULL);

    return 0;
}
