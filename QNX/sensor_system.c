#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int dashboard_sock = -1;
struct sockaddr_in dashboard_addr;

void init_dashboard_udp() {
    dashboard_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&dashboard_addr, 0, sizeof(dashboard_addr));
    dashboard_addr.sin_family = AF_INET;
    dashboard_addr.sin_port = htons(5000);
    dashboard_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
}

void send_dashboard_data(int faults, int latency, const char* msg_text) {
    if (dashboard_sock == -1) init_dashboard_udp();
    char json[512];
    snprintf(json, sizeof(json), "{\"latency_ns\": %d, \"faults\": %d, \"text\": \"%s\"}", latency, faults, msg_text);
    sendto(dashboard_sock, json, strlen(json), 0, (struct sockaddr*)&dashboard_addr, sizeof(dashboard_addr));
}

#define NUM_SENSORS 3
#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t lock;

void* sensor(void* arg) {
    int id = *(int*)arg;

    while (1) {
        int data = rand() % 100;

        // Fault injection simulation
        if (rand() % 10 == 0) {
            printf("Sensor %d FAILED\n", id);
            char buf[64];
            snprintf(buf, sizeof(buf), "Sensor %d FAILED", id);
            send_dashboard_data(1, 0, buf);
            sleep(1);
            continue;
        }

        pthread_mutex_lock(&lock);

        if (count < BUFFER_SIZE) {
            buffer[count++] = data;
            printf("Sensor %d -> pushed: %d\n", id, data);
            char buf[64];
            snprintf(buf, sizeof(buf), "Sensor %d -> pushed: %d", id, data);
            send_dashboard_data(0, rand()%50, buf);
        }

        pthread_mutex_unlock(&lock);

        sleep(1);
    }
}

void* aggregator(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (count > 0) {
            int data = buffer[--count];
            printf("Aggregator -> processed: %d\n", data);
            char buf[64];
            snprintf(buf, sizeof(buf), "Aggregator -> processed: %d", data);
            send_dashboard_data(0, 15, buf);
        }

        pthread_mutex_unlock(&lock);

        sleep(2);
    }
}

void* logger(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        printf("Logger -> buffer size: %d\n", count);
        pthread_mutex_unlock(&lock);

        sleep(3);
    }
}

int main() {
    pthread_t sensors[NUM_SENSORS], agg, log;
    int ids[NUM_SENSORS];

    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < NUM_SENSORS; i++) {
        ids[i] = i + 1;
        pthread_create(&sensors[i], NULL, sensor, &ids[i]);
    }

    pthread_create(&agg, NULL, aggregator, NULL);
    pthread_create(&log, NULL, logger, NULL);

    for (int i = 0; i < NUM_SENSORS; i++) {
        pthread_join(sensors[i], NULL);
    }

    pthread_join(agg, NULL);
    pthread_join(log, NULL);

    pthread_mutex_destroy(&lock);

    return 0;
}