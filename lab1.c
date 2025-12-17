// $output="./build/lab1"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define TOTAL_EVENTS 5

typedef struct {
    int event_id;
    char payload[64];
} Event;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int data_ready = 0;
static Event* current_event = NULL;

void* event_producer(void* unused) {
    for (int idx = 1; idx <= TOTAL_EVENTS; ++idx) {
        sleep(1);

        pthread_mutex_lock(&mtx);

        if (data_ready) {
            pthread_mutex_unlock(&mtx);
            continue;
        }

        current_event = (Event*)malloc(sizeof(Event));
        current_event->event_id = idx;
        snprintf(current_event->payload, sizeof(current_event->payload),
                 "Event_%d", idx);

        data_ready = 1;
        printf("Provider: Created event (ID=%d, msg='%s')\n",
               current_event->event_id, current_event->payload);

        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

void* event_handler(void* unused) {
    for (int idx = 1; idx <= TOTAL_EVENTS; ++idx) {
        pthread_mutex_lock(&mtx);

        while (!data_ready) {
            pthread_cond_wait(&cv, &mtx);
        }

        if (current_event != NULL) {
            printf("Consumer: Handled event (ID=%d, msg='%s')\n",
                   current_event->event_id, current_event->payload);
            free(current_event);
            current_event = NULL;
        }

        data_ready = 0;
        pthread_mutex_unlock(&mtx);
    }

    return NULL;
}

int main() {
    pthread_t producer_tid, consumer_tid;

    pthread_create(&producer_tid, NULL, event_producer, NULL);
    pthread_create(&consumer_tid, NULL, event_handler, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    pthread_mutex_destroy(&mtx);
    pthread_cond_destroy(&cv);

    printf("Main: Execution complete.\n");
    return 0;
}
