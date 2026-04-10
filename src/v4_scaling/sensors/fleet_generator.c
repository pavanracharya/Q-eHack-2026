#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <time.h>
#include "../common/adas_ipc.h"
#include "../common/zonal_config.h"

// ---------------------------------------------------------
// FLEET GENERATOR (Simulates 100 Sensors)
// ---------------------------------------------------------
void* fleet_generator_start(void* arg) {
    int coids[NUM_ZONES];
    char *zone_names[NUM_ZONES] = {
        ATTACH_ZONE_VISION, ATTACH_ZONE_RADAR, 
        ATTACH_ZONE_PROX, ATTACH_ZONE_VITALS
    };

    // 1. Establish connections to all 4 Zonal Aggregators
    for (int i = 0; i < NUM_ZONES; i++) {
        coids[i] = name_open(zone_names[i], 0);
        while (coids[i] == -1) {
            printf("[FLEET] Waiting for Aggregator %s...\n", zone_names[i]);
            sleep(1);
            coids[i] = name_open(zone_names[i], 0);
        }
    }

    printf("[FLEET] Connected to all zones. Starting 100-sensor simulation stream...\n");

    raw_sensor_msg_t msg;
    msg.type = MSG_RAW_DATA;
    adas_reply_t reply;

    int cycle_count = 0;
    while (1) {
        cycle_count++;
        for (int sid = 0; sid < TOTAL_SENSORS; sid++) {
            const sensor_config_t *config = &HARDCODED_FLEET[sid];
            int zone = config->target_zone;

            // SCENARIO 5: Sensor Failure (Drop Zone Proximity every 15 cycles)
            if (zone == ZONE_PROXIMITY && (cycle_count % 15 == 0)) continue;

            msg.sensor_id = sid;
            snprintf(msg.sensor_name, sizeof(msg.sensor_name), "%s", config->name);
            msg.timestamp_ns = (uint64_t)time(NULL) * 1000000000ULL;
            msg.value = (float)(rand() % 100);
            msg.confidence = 90 + (rand() % 10);

            // SCENARIO 1: Time Misalignment (Sensor ID 3)
            if (sid == 3 && (cycle_count % 5 == 0)) msg.timestamp_ns -= 150000000ULL;

            // SCENARIO 2: Conflict (Vision vs Radar)
            if (cycle_count % 10 == 0) {
                if (sid == 0) msg.value = 80.0f;
                if (sid == 12) msg.value = 10.0f;
            }

            printf("Sensor %d (%s): Sending data via simulated MsgSend...\n", sid, msg.sensor_name);
            MsgSend(coids[zone], &msg, sizeof(msg), &reply, sizeof(reply));
        }
        usleep(1000000); 
    }

    return NULL;
}
