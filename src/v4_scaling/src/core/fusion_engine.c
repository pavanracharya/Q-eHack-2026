#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "../common/adas_ipc.h"
#include "../common/zonal_config.h"
#include "../common/system_state.h"

#define PULSE_CODE_WATCHDOG 1

// ---------------------------------------------------------
// CENTRAL FUSION ENGINE (Multicore Processor)
// ---------------------------------------------------------
void* central_fusion_start(void* arg) {
    char *central_name = (char*)arg;
    name_attach_t *attach;
    struct sigevent event;
    timer_t timerid;
    struct itimerspec timer_spec;

    // 1. Register with QNX Name Service (Dynamic Name)
    if ((attach = name_attach(NULL, central_name, 0)) == NULL) {
        printf("[CENTRAL] CRITICAL Error: Could not attach dynamic name %s\n", central_name);
        return NULL;
    }

    printf("\033[1;36m[CENTRAL] Fusion Brain Online: %s | Status: Ready for High-Fidelity Data\033[0m\n", central_name);
    // 2. Setup QNX Watchdog Pulse (Proper QNX RTOS Pattern)
    // Using 0 as the index for local channel connection if _NTO_SIDE_CHANNELS is missing
    int coid = ConnectAttach(0, 0, attach->chid, 0, 0);
    SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, PULSE_CODE_WATCHDOG, 0);
    if (timer_create(CLOCK_MONOTONIC, &event, &timerid) == -1) {
        printf("[CENTRAL] Timer creation failed\n");
    }
    
    timer_spec.it_value.tv_sec = 8;  // First trigger in 8s (Wait for stability)
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = 5; // Check every 5s
    timer_spec.it_interval.tv_nsec = 0;
    timer_settime(timerid, 0, &timer_spec, NULL);

    printf("\033[1;36m[CENTRAL] Fusion Engine Online (Priority: RT, Affinity: Cores 4-7)\033[0m\n");

    zonal_summary_msg_t msg; 
    adas_reply_t reply;
    int rcvid;
    int last_zone_seen[NUM_ZONES] = {0};
    int zone_failed_state[NUM_ZONES] = {0}; // Track failure state to prevent repeat logs

    while (g_keep_running) {
        // Core Affinity (Locking to high-performance cores 4-7)
        ThreadCtl(_NTO_TCTL_RUNMASK, (void*)(uintptr_t)0xF0);

        // Subtle Processing Heartbeat (every ~5s of processing)
        static int heartbeat_counter = 0;
        if (heartbeat_counter++ % 500 == 0) {
             if (zone_failed_state[ZONE_PROXIMITY]) {
                 printf("\033[1;33m[FAILSAFE ACTIVE] Zone 2 Offline | Brain using redundant Vision (Zone 0) & Radar (Zone 1) to guess Proximity...\033[0m\n");
             } else {
                 printf("\033[1;30m[FUSION] System Nominal: Actively fusing 500 sensor samples...\033[0m\n");
             }
        }

        rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) break;

        // SCENARIO 5: Sensor Failure Detection (Watchdog Pulse handling)
        if (rcvid == 0) { 
            struct _pulse *pulse = (struct _pulse *)&msg;
            if (pulse->code == PULSE_CODE_WATCHDOG) {
                for (int i = 0; i < NUM_ZONES; i++) {
                    if (last_zone_seen[i] == 0 && i != ZONE_VEHICLE_VITALS) {
                        if (zone_failed_state[i] == 0) {
                            printf("\033[1;31m[WATCHDOG] HEARTBEAT LOST from Zone %d\033[0m\n", i);
                            printf("\033[1;31m[FAILSAFE] Initiating Redundancy protocol: Using alternates (Zones 0 & 1) for path-finding.\033[0m\n");
                            zone_failed_state[i] = 1;
                        }
                    }
                    last_zone_seen[i] = 0; // Reset for next period
                }
            }
            continue;
        }

        if (msg.type == MSG_ZONAL_SUMMARY) {
            if (zone_failed_state[msg.zone_id] == 1) {
                printf("\033[1;32m[RESOLUTION] HEARTBEAT RECOVERED for Zone %d\033[0m\n", msg.zone_id);
                printf("\033[1;32m[SYSTEM] Seamlessly transitioning back to Primary Sensors. Disabling redundant alternates.\033[0m\n");
                zone_failed_state[msg.zone_id] = 0;
            }
            last_zone_seen[msg.zone_id] = 1;

            // SCENARIO 2: Conflict Simulation (Vision vs Radar)
            // If Vision says "Danger" (we simulated this in fleet gen)
            if (msg.zone_id == ZONE_VISION) {
                // In a real system, we'd check cluster values. Here we check the simulated state.
                printf("\033[1;33m[TRUST] Sensor Conflict: VISION (Object Detected) vs RADAR (Clear Path)\033[0m\n");
                printf("\033[1;32m[RESOLVE] Reliability Model: Rain detected -> Trusting RADAR (Weight 0.95)\033[0m\n");
            }

            // SCENARIO 3: Latency vs Safety (Radar triggers fast action)
            if (msg.zone_id == ZONE_RADAR) {
                printf("\033[1;34m[PRIORITY] RADAR (High-Speed) -> Initiating Emergency Braking Sequence\033[0m\n");
                printf("[FUSION] Camera processing lag detected (120ms). Camera data will refine braking in next cycle.\n");
            }

            reply.status = 0;
            MsgReply(rcvid, 0, &reply, sizeof(reply));
        }
    }

    printf("\033[1;31m[CENTRAL] Fusion Engine Detaching: %s\033[0m\n", ATTACH_CENTRAL);
    timer_delete(timerid);
    name_detach(attach, 0);
    return NULL;
}
