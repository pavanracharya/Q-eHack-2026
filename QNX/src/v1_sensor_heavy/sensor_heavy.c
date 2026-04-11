#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define NUM_SENSORS 4
#define MSG_QUEUE_SIZE 10

/* 
 * QNX Message Passing Simulation
 * Simulates MsgSend/MsgReceive channel using POSIX primitives 
 * for shared-environment compatibility.
 */

typedef struct {
    int sensor_id;
    double heavy_computation_result;
} SensorMsg_t;

// Channel structure for IPC emulation
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

// Emulates QNX MsgSend() behavior (blocking send)
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

// Emulates QNX MsgReceive() behavior (blocking receive)
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

// Simulated heavy workload for performance benchmarking
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
         * NOTE: In native QNX, thread priorities would be set here 
         * using pthread_setschedparam() with SCHED_RR or SCHED_FIFO.
         */

    while (1) {
        printf("Sensor %d: Starting heavy computation (Simulating CPU load)...\n", id);
        
        // This will block the CPU heavily
        double data = perform_heavy_computation();
        
        SensorMsg_t msg;
        msg.sensor_id = id;
        msg.heavy_computation_result = data;
        
        printf("Sensor %d: Sending data via simulated MsgSend...\n", id);
        MsgSend_Sim(&channel, msg);
        
        // Sleep a bit before the next read
        usleep(500000); // 500ms sleep
    }
    return NULL;
}

void* aggregator_thread(void* arg) {
    while (1) {
        /* 
         * Native QNX Resource Manager pattern: 
         * MsgReceive() blocks until a message is sent to the channel.
         */
        SensorMsg_t msg = MsgReceive_Sim(&channel);
        
        printf("Aggregator (Server): Received from Sensor %d | Data: %f\n", 
               msg.sensor_id, msg.heavy_computation_result);
    }
    return NULL;
}

int main() {
    pthread_t sensors[NUM_SENSORS], aggregator;
    int ids[NUM_SENSORS];

    channel_init(&channel);

    printf("Starting QNX RTOS Simulated Sensor System (Heavy CPU Load)\n");

    // Launch server and client threads
    pthread_create(&aggregator, NULL, aggregator_thread, NULL);

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
