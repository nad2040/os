#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<pthread.h>
#include<unistd.h>

#define ESC "\033"
#define CSI "["
#define PREV_LINE "F"
#define BACK "D"

const int PROG_BAR_LEN = 40;
const int NUM_THREADS = 5;


typedef struct {
    int count_to_val;
    int progress;
    pthread_t thread;
} thread_info;


void update_bar(thread_info *info) {
    int num_chars = info->progress * 1000 / info->count_to_val * PROG_BAR_LEN / 1000;
    printf("[");
    for (int i = 0; i < num_chars; i++) {
        printf("#");
    }
    if (info->progress < info->count_to_val) {
        printf(ESC CSI BACK "|");
    }
    for (int i = num_chars; i < PROG_BAR_LEN; i++) {
        printf(" ");
    }
    printf("]\n");

}

void *thread_func(void *arg) {
    thread_info *info = (thread_info *)arg;

    for (info->progress = 0;
         info->progress < info->count_to_val;
         info->progress++) {
        usleep(100000);
    }

    return NULL;
}

int main() {
    thread_info threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = (thread_info){ .count_to_val = 100, .progress = 0 };
        pthread_create(&threads[i].thread, NULL, thread_func, &threads[i]);
    }

    bool done = false;
    while (!done) {
        done = true;

        for (int i = 0; i < NUM_THREADS; i++) {
            update_bar(&threads[i]);
            if (threads[i].progress < threads[i].count_to_val) {
                done = false;
            }
        }
        if (!done) {
            printf(ESC CSI "%d" PREV_LINE, NUM_THREADS);
        }
        usleep(100000);
    }

}
