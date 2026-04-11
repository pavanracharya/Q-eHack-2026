#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include "../common/zonal_config.h"

// Forward Declarations
void* zonal_aggregator_start(void* arg);
void* central_fusion_start(void* arg);
void* fleet_generator_start(void* arg);

int main() {
    printf("\033[1;37m============================================================\033[0m\n");
    printf("\033[1;36m       QNX RTOS v4: SCALABLE ADAS SENSOR AGGREGATOR         \033[0m\n");
    printf("\033[1;34m         [ 8-CORE AFFINITY | 100-SENSOR REAL-TIME ]         \033[0m\n");
    printf("\033[1;37m============================================================\033[0m\n");

    pthread_t zones[NUM_ZONES], central_th, generator_th;
    int zone_ids[NUM_ZONES] = {ZONE_VISION, ZONE_RADAR, ZONE_PROXIMITY, ZONE_VEHICLE_VITALS};
    
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    // 1. Launch 4 Zonal Aggregators (Cores 1-4)
    for (int i = 0; i < NUM_ZONES; i++) {
        param.sched_priority = 20;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&zones[i], &attr, zonal_aggregator_start, &zone_ids[i]);
    }

    // 2. Launch Central Fusion Engine (Cores 5-6)
    param.sched_priority = 22;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&central_th, &attr, central_fusion_start, NULL);

    // 3. Launch the 100-Sensor Fleet Generator (Core 7)
    param.sched_priority = 15;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&generator_th, &attr, fleet_generator_start, NULL);

    printf("[SYSTEM] 100-Sensor scaling active across 8 Cores.\n");
    printf("[SYSTEM] Hierarchy: [Sensors] -> [Zones] -> [Central Fusion]\n");

    // Joining one to keep main alive
    pthread_join(central_th, NULL);

    return 0;
}
