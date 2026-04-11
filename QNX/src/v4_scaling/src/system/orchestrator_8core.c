#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <sched.h>
#include "../common/zonal_config.h"
#include "../common/system_state.h"

// System State
volatile int g_keep_running = 1;

void handle_shutdown(int sig) {
    printf("\n\033[1;31m[SYSTEM] Signal received. Shutting down and detaching QNX names...\033[0m\n");
    g_keep_running = 0;
}

// Worker Threads
void* zonal_aggregator_start(void* arg);
void* central_fusion_start(void* arg);
void* fleet_generator_start(void* arg);

int main() {
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    printf("\033[1;37m============================================================\033[0m\n");
    printf("\033[1;36m       QNX RTOS v4: SCALABLE ADAS SENSOR AGGREGATOR         \033[0m\n");
    printf("\033[1;34m         [ 8-CORE AFFINITY | 100-SENSOR REAL-TIME ]         \033[0m\n");
    printf("\033[1;37m============================================================\033[0m\n");

    // Unique session identifiers for dynamic IPC
    int session_pid = getpid();
    char z_vision[50], z_radar[50], z_prox[50], z_vitals[50], z_central[50];
    
    snprintf(z_vision, 50, "%s_%d", ATTACH_ZONE_VISION, session_pid);
    snprintf(z_radar, 50, "%s_%d", ATTACH_ZONE_RADAR, session_pid);
    snprintf(z_prox, 50, "%s_%d", ATTACH_ZONE_PROX, session_pid);
    snprintf(z_vitals, 50, "%s_%d", ATTACH_ZONE_VITALS, session_pid);
    snprintf(z_central, 50, "%s_%d", ATTACH_CENTRAL, session_pid);

    char *naming_map[5] = {z_vision, z_radar, z_prox, z_vitals, z_central};

    printf("[SYSTEM] Launching Session ID: %d (Dynamic IPC active)\n", session_pid);

    pthread_t zones[NUM_ZONES], central_th, generator_th;
    int zone_ids[NUM_ZONES] = {ZONE_VISION, ZONE_RADAR, ZONE_PROXIMITY, ZONE_VEHICLE_VITALS};
    
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    // Zonal Aggregators (Cores 0-3)
    for (int i = 0; i < NUM_ZONES; i++) {
        param.sched_priority = 20;
        pthread_attr_setschedparam(&attr, &param);
        pthread_create(&zones[i], &attr, zonal_aggregator_start, (void*)naming_map[i]);
        usleep(100000); // Small stagger for QNX GNS stability
    }

    // Central Fusion Engine (Cores 4-7)
    param.sched_priority = 22;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&central_th, &attr, central_fusion_start, (void*)z_central);

    // Fleet Data Generator
    param.sched_priority = 15;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&generator_th, &attr, fleet_generator_start, (void*)session_pid);

    printf("[SYSTEM] 100-Sensor scaling active across 8 Cores.\n");
    printf("[SYSTEM] Hierarchy: [Sensors] -> [Zones] -> [Central Fusion]\n");

    // Joining one to keep main alive
    pthread_join(central_th, NULL);

    return 0;
}
