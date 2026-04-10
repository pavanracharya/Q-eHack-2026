#ifndef ADAS_IPC_H
#define ADAS_IPC_H

#include <stdint.h>
#include <sys/neutrino.h>
#include "zonal_config.h"

// ---------------------------------------------------------
// MESSAGE TYPES
// ---------------------------------------------------------
typedef enum {
    MSG_RAW_DATA = 0x200,    // Sensor -> Zone
    MSG_ZONAL_SUMMARY,       // Zone -> Central
    MSG_CONTROL_THROTTLE     // Central -> Zone
} ScaledMsgType_e;

// ---------------------------------------------------------
// ZONAL MESSAGE STRUCTURES
// ---------------------------------------------------------

// Raw data from one of the 100 sensors
typedef struct {
    uint16_t type;
    uint16_t sensor_id;
    char     sensor_name[20]; // Added for verbose trace
    uint64_t timestamp_ns;
    float    value;           // Simulated data point (Speed, Dist, etc)
    uint8_t  confidence;
} raw_sensor_msg_t;

// Condensed summary from a Zone Aggregator (e.g. Radar Zone)
typedef struct {
    uint16_t type;
    uint16_t zone_id;
    uint16_t object_count;    // Count of clusters found in this zone
    uint16_t env_context;     // Added: 0=Clear, 1=Rain, 2=Fog (for Trust logic)
    struct {
        float x, y;
        float velocity;
        float danger_score;   // 0.0 - 1.0 based on Zonal filtering
    } clusters[10];
} zonal_summary_msg_t;

typedef struct {
    int status;
} adas_reply_t;

#endif // ADAS_IPC_H
