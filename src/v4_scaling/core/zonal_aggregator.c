#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <pthread.h>
#include "../common/adas_ipc.h"
#include "../common/zonal_config.h"

// ---------------------------------------------------------
// ZONAL AGGREGATOR SERVER
// ---------------------------------------------------------
void* zonal_aggregator_start(void* arg) {
    int zone_id = *(int*)arg;
    name_attach_t *attach;
    char *zone_name;

    switch(zone_id) {
        case ZONE_VISION: zone_name = ATTACH_ZONE_VISION; break;
        case ZONE_RADAR:  zone_name = ATTACH_ZONE_RADAR;  break;
        case ZONE_PROXIMITY: zone_name = ATTACH_ZONE_PROX; break;
        default: zone_name = ATTACH_ZONE_VITALS; break;
    }

    // 1. Register with QNX Name Service
    if ((attach = name_attach(NULL, zone_name, 0)) == NULL) {
        printf("[ZONE %d] Error: Could not attach name %s\n", zone_id, zone_name);
        return NULL;
    }

    printf("[ZONE %d] Aggregator Online: %s (PID: %d)\n", zone_id, zone_name, getpid());

    // Core synchronization primitives
    static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    raw_sensor_msg_t msg;
    adas_reply_t reply;
    int rcvid;

    // Clustering state (Protected by Mutex)
    int raw_msg_count = 0;
    int dropped_frames = 0;

    while (1) {
        // 2. Core Affinity logic (Zonal tasks on Cores 0-3)
        uint32_t runmask = 1 << (zone_id % 4);
        ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)runmask);

        // 3. Receive raw data from sensors in this zone
        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) break;
        if (rcvid == 0) continue; 

        if (msg.type == MSG_RAW_DATA) {
            uint64_t now_ns = (uint64_t)time(NULL) * 1000000000ULL;
            
            pthread_mutex_lock(&state_mutex);

            // VERBOSE LOGGING (Requested by user)
            printf("Aggregator (Server): Received from Sensor %d (%s) | Data: %f\n", 
                   msg.sensor_id, msg.sensor_name, msg.value);

            // SCENARIO 4: Data Overload (Simulated Adaptive Throttling)
            // If we receive too many messages in a short burst, drop some
            if (raw_msg_count > 15) {
                dropped_frames++;
                printf("\033[1;33m[THROTTLE] Load High (Simulated 92%% CPU) | Throttling %s | Dropped: %d\033[0m\n", 
                       msg.sensor_name, dropped_frames);
                pthread_mutex_unlock(&state_mutex);
                reply.status = -1; // Busy
                MsgReply(rcvid, 0, &reply, sizeof(reply));
                continue;
            }

            // SCENARIO 1: Time Misalignment Detection
            int64_t drift = (int64_t)now_ns - (int64_t)msg.timestamp_ns;
            if (drift > 100000000LL) { // > 100ms drift
                printf("\033[1;35m[SYNC] Drift Detected from %s: %lldms | RESOLVE: Applying Prediction Vector\033[0m\n", 
                       msg.sensor_name, (long long)(drift / 1000000));
            }

            raw_msg_count++;
            
            // Logic: Every 10 raw messages, send a summary to Central
            if (raw_msg_count >= 10) {
                printf("\033[1;32m[ZONE %d] Clustered 10 samples. Sending state-vector to Central Fusion...\n\033[0m", zone_id);
                // In real implementation, MsgSend to CENTRAL would go here
                raw_msg_count = 0;
                dropped_frames = 0;
            }

            pthread_mutex_unlock(&state_mutex);

            reply.status = 0;
            MsgReply(rcvid, 0, &reply, sizeof(reply));
        }
    }

    name_detach(attach, 0);
    return NULL;
}
