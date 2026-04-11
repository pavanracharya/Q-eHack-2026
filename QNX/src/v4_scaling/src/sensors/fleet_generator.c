#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <time.h>
#include "../common/adas_ipc.h"
#include "../common/zonal_config.h"
#include "../common/system_state.h"

// Fleet Generator - Simulates 100 Sensors
void* fleet_generator_start(void* arg) {
    int session_pid = (intptr_t)arg;
    int coids[NUM_ZONES];
    char *base_names[NUM_ZONES] = {
        ATTACH_ZONE_VISION, ATTACH_ZONE_RADAR, 
        ATTACH_ZONE_PROX, ATTACH_ZONE_VITALS
    };
    char dyn_name[64];

    // Establish connections to Zonal Aggregators
    for (int i = 0; i < NUM_ZONES; i++) {
        snprintf(dyn_name, 64, "%s_%d", base_names[i], session_pid);
        coids[i] = name_open(dyn_name, 0);
        while (coids[i] == -1) {
            printf("[FLEET] Connecting to Session %d | Target: %s...\n", session_pid, dyn_name);
            sleep(1);
            coids[i] = name_open(dyn_name, 0);
        }
    }

    printf("[FLEET] Connected to all dynamic zones. Starting 100-sensor stream...\n");

    raw_sensor_msg_t msg;
    msg.type = MSG_RAW_DATA;
    adas_reply_t reply;

    int cycle_count = 0;
    while (g_keep_running) {
        cycle_count++;
        
        if (cycle_count == 1) {
            printf("\033[1;33m[FLEET] Phase 1: Warming up sensors (Stabilizing baseline)...\033[0m\n");
        } else if (cycle_count == 4) {
             printf("\033[1;32m[FLEET] Phase 2: System Stabilized. Commencing safety-critical injections.\033[0m\n");
        }

        for (int sid = 0; sid < TOTAL_SENSORS; sid++) {
            const sensor_config_t *config = &HARDCODED_FLEET[sid];
            int zone = config->target_zone;

            // Simulating sensor hardware failures
            int failure_cycle_start = 15;
            int failure_duration = 6;
            
            if (zone == ZONE_PROXIMITY && cycle_count >= failure_cycle_start && 
               ((cycle_count - failure_cycle_start) % 20 < failure_duration)) {
                
                // if (sid == 32) {
                //    printf("\033[1;31m[SCENARIO] >>> ZONE 2 POWER FAILURE DETECTED (Cycle %d) <<<\033[0m\n", cycle_count);
                //    printf("\033[1;30m[FLEET] Zone 2 (Proximity) is offline. Watchdog timer is counting down...\033[0m\n");
                // }
                continue;
            }

            msg.sensor_id = sid;
            snprintf(msg.sensor_name, sizeof(msg.sensor_name), "%s", config->name);
            msg.timestamp_ns = (uint64_t)time(NULL) * 1000000000ULL;
            msg.value = (float)(rand() % 100);
            msg.confidence = 90 + (rand() % 10);

            // Fault injections only after warm-up (cycle 4+)
            if (cycle_count >= 4) {
                // Simulated fault injections
                if (sid == 3 && (cycle_count % 7 == 0)) msg.timestamp_ns -= 150000000ULL;

                // Simulated sensor conflicts
                if ((cycle_count + 2) % 10 == 0) {
                    if (sid == 0) msg.value = 80.0f;  // Vision sees object
                    if (sid == 12) msg.value = 10.0f; // Radar sees clear
                }
            }

            // printf("Sensor %d (%s): Sending data via simulated MsgSend...\n", sid, msg.sensor_name);
            MsgSend(coids[zone], &msg, sizeof(msg), &reply, sizeof(reply));
        }
        usleep(1000000); 
    }

    return NULL;
}
