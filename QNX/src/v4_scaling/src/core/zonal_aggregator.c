#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "../common/adas_ipc.h"
#include "../common/zonal_config.h"
#include "../common/system_state.h"

// Zonal Aggregator - IPC Server
void* zonal_aggregator_start(void* arg) {
    char *zone_name = (char*)arg;
    name_attach_t *attach;
    int zone_id = 0;

    // Detect Zone ID from name prefix
    if (strstr(zone_name, ATTACH_ZONE_VISION)) zone_id = ZONE_VISION;
    else if (strstr(zone_name, ATTACH_ZONE_RADAR)) zone_id = ZONE_RADAR;
    else if (strstr(zone_name, ATTACH_ZONE_PROX)) zone_id = ZONE_PROXIMITY;
    else zone_id = ZONE_VEHICLE_VITALS;

    // Register with QNX Name Service
    if ((attach = name_attach(NULL, zone_name, 0)) == NULL) {
        printf("[ZONE %d] CRITICAL Error: Could not attach dynamic name %s\n", zone_id, zone_name);
        return NULL;
    }

    printf("\033[1;33m[ZONE %d] Aggregator Online: %s (PID: %d) | Status: Awaiting Fleet Init...\033[0m\n", 
           zone_id, zone_name, getpid());

    // Core synchronization primitives
    static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    raw_sensor_msg_t msg;
    adas_reply_t reply;
    int rcvid;

    // Clustering state (Protected by Mutex)
    int raw_msg_count = 0;
    int dropped_frames = 0;

    while (g_keep_running) {
        // 2. Core Affinity logic (Zonal tasks on Cores 0-3)
        uint32_t runmask = 1 << (zone_id % 4);
        ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)runmask);

        // IPC Receive
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) break;
        if (rcvid == 0) continue; 

        if (msg.type == MSG_RAW_DATA) {
            uint64_t now_ns = (uint64_t)time(NULL) * 1000000000ULL;
            
            pthread_mutex_lock(&state_mutex);

            // Thread safety for clustering state

            // Adaptive Throttling
            if (raw_msg_count > 15) {
                dropped_frames++;
                printf("\033[1;33m[THROTTLE] Load High (Simulated 92%% CPU) | Throttling %s | Dropped: %d\033[0m\n", 
                       msg.sensor_name, dropped_frames);
                pthread_mutex_unlock(&state_mutex);
                reply.status = -1; // Busy
                MsgReply(rcvid, 0, &reply, sizeof(reply));
                continue;
            }

            // Time Alignment & Drift Detection
            static int sensor_drift_active[TOTAL_SENSORS] = {0};
            int64_t drift = (int64_t)now_ns - (int64_t)msg.timestamp_ns;
            
            if (drift > 100000000LL) { // > 100ms drift
                if (sensor_drift_active[msg.sensor_id] == 0) {
                    // printf("\033[1;35m[SYNC] Drift Detected from %s: %lldms | RESOLVE: Applying Prediction Vector\033[0m\n", 
                    //        msg.sensor_name, (long long)(drift / 1000000));
                    sensor_drift_active[msg.sensor_id] = 1;
                }
            } else {
                sensor_drift_active[msg.sensor_id] = 0; // Recovered
            }

            raw_msg_count++;
            
            // Logic: Every 10 raw messages, send a summary to Central
            if (raw_msg_count >= 10) {
                // printf("\033[1;32m[ZONE %d] Clustered 10 samples. Sending state-vector to Central Fusion...\n\033[0m", zone_id);
                // In real implementation, MsgSend to CENTRAL would go here
                raw_msg_count = 0;
                dropped_frames = 0;
            }

            pthread_mutex_unlock(&state_mutex);

            reply.status = 0;
            MsgReply(rcvid, 0, &reply, sizeof(reply));
        }
    }

    printf("\033[1;31m[ZONE %d] Aggregator Detaching: %s\033[0m\n", zone_id, zone_name);
    name_detach(attach, 0);
    return NULL;
}
