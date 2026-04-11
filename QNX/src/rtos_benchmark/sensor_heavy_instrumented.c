#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define NUM_SENSORS 4
#define MSG_QUEUE_SIZE 10

/* 
 * QNX Message Passing Simulation (Instrumented)
 * Benchmarks MsgSend/MsgReceive latency using POSIX primitives.
 */

typedef struct {
    int sensor_id;
    double heavy_computation_result;
    struct timespec send_time; // ADDED: Used to benchmark IPC passing latency
} SensorMsg_t;

// Helper function to calculate time difference in nanoseconds
long calc_time_diff_ns(struct timespec start, struct timespec end) {
    long sec_diff = end.tv_sec - start.tv_sec;
    long nsec_diff = end.tv_nsec - start.tv_nsec;
    return (sec_diff * 1000000000L) + nsec_diff;
}

// Instrumented IPC Channel
typedef struct {
    SensorMsg_t queue[MSG_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond_full;
    pthread_cond_t cond_empty;
} MsgChannel_t;

MsgChannel_t channel;

void channel_init(MsgChannel_t* ch) {
    ch->head = 0;
    ch->tail = 0;
    ch->count = 0;
    pthread_mutex_init(&ch->mutex, NULL);
    pthread_cond_init(&ch->cond_full, NULL);
    pthread_cond_init(&ch->cond_empty, NULL);
}

// Emulates blocking MsgSend() with telemetry capture
void MsgSend_Sim(MsgChannel_t* ch, SensorMsg_t msg) {
    pthread_mutex_lock(&ch->mutex);
    while (ch->count == MSG_QUEUE_SIZE) {
        pthread_cond_wait(&ch->cond_empty, &ch->mutex);
    }
    ch->queue[ch->tail] = msg;
    ch->tail = (ch->tail + 1) % MSG_QUEUE_SIZE;
    ch->count++;
    pthread_cond_signal(&ch->cond_full);
    pthread_mutex_unlock(&ch->mutex);
}

// Emulates blocking MsgReceive()
SensorMsg_t MsgReceive_Sim(MsgChannel_t* ch) {
    pthread_mutex_lock(&ch->mutex);
    while (ch->count == 0) {
        pthread_cond_wait(&ch->cond_full, &ch->mutex);
    }
    SensorMsg_t msg = ch->queue[ch->head];
    ch->head = (ch->head + 1) % MSG_QUEUE_SIZE;
    ch->count--;
    pthread_cond_signal(&ch->cond_empty);
    pthread_mutex_unlock(&ch->mutex);
    return msg;
}

// Heavy CPU workload function
double perform_heavy_computation() {
    double result = 0.0;
    // Consume CPU by calculating square roots and trigonometric functions in a large loop
    for (int i = 1; i < 5000000; i++) {
        result += sqrt((double)i) * sin((double)i);
    }
    return result;
}

void* sensor_thread(void* arg) {
    int id = *(int*)arg;
    
        /* 
         * Native QNX: Priority and affinity would be set here.
         */

    while (1) {
        printf("Sensor %d: Starting heavy computation (Simulating CPU load)...\n", id);
        
        // This will block the CPU heavily
        double data = perform_heavy_computation();
        
        SensorMsg_t msg;
        msg.sensor_id = id;
        msg.heavy_computation_result = data;
        
        // TELEMETRY: Record exact time before sending message
        clock_gettime(CLOCK_MONOTONIC, &msg.send_time);
        
        // printf("Sensor %d: Sending data...\n", id); // Removed to avoid print lag
        MsgSend_Sim(&channel, msg);
        
        // Sleep a bit before the next read
        usleep(10000); // 10ms sleep instead of 500ms to stress test the scheduler
    }
    return NULL;
}

void* aggregator_thread(void* arg) {
    // Telemetry capture thread
    struct timespec recv_time;
    
    while (1) {
        // Wait for IPC message
        SensorMsg_t msg = MsgReceive_Sim(&channel);
        
        // TELEMETRY: Record exact time when message is acquired
        clock_gettime(CLOCK_MONOTONIC, &recv_time);
        
        long latency_ns = calc_time_diff_ns(msg.send_time, recv_time);
        
        printf("[Aggregator] RECV Sensor %d | Data: %.2f | Wake-Up Latency: %ld ns \n", 
               msg.sensor_id, msg.heavy_computation_result, latency_ns);
               
        /* 
         * MsgReceive completes here. Native QNX would use MsgReply() to finish.
         */
    }
    return NULL;
}

int main() {
    pthread_t sensors[NUM_SENSORS], aggregator;
    int ids[NUM_SENSORS];

    channel_init(&channel);

    printf("Starting QNX RTOS Simulated Sensor System (Heavy CPU Load)\n");

    // The aggregator acts as our QNX Server Channel owner
    pthread_create(&aggregator, NULL, aggregator_thread, NULL);

    // The sensors act as clients connecting and sending messages to the server
    for (int i = 0; i < NUM_SENSORS; i++) {
        ids[i] = i + 1;
        pthread_create(&sensors[i], NULL, sensor_thread, &ids[i]);
    }

    // Wait for threads (in this continuous system, they run indefinitely)
    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(sensors[i], NULL);
    }
    pthread_join(aggregator, NULL);

    return 0;
}
