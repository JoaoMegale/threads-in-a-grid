#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
// #include <linux/time.h>

#define MAX_N 20

typedef struct {
    int x, y, time;
} Position;

typedef struct {
    int id;
    int group_id;
    int num_positions;
    Position *positions;
} ThreadData;

int N, n_threads;
ThreadData threads[MAX_N];
pthread_mutex_t board_mutexes[MAX_N][MAX_N];
pthread_cond_t board_conds[MAX_N][MAX_N];

void passa_tempo(int tid, int x, int y, int decimos) {
    struct timespec zzz, agora;
    static struct timespec inicio = {0, 0};
    int tstamp;

    if (inicio.tv_sec == 0 && inicio.tv_nsec == 0) {
        clock_gettime(CLOCK_REALTIME, &inicio);
    }

    zzz.tv_sec = decimos / 10;
    zzz.tv_nsec = (decimos % 10) * 100L * 1000000L;

    clock_gettime(CLOCK_REALTIME, &agora);
    tstamp = (10 * agora.tv_sec + agora.tv_nsec / 100000000L) -
             (10 * inicio.tv_sec + inicio.tv_nsec / 100000000L);

    printf("%3d [ %2d @(%2d,%2d) z%4d\n", tstamp, tid, x, y, decimos);
    nanosleep(&zzz, NULL);

    clock_gettime(CLOCK_REALTIME, &agora);
    tstamp = (10 * agora.tv_sec + agora.tv_nsec / 100000000L) -
             (10 * inicio.tv_sec + inicio.tv_nsec / 100000000L);

    printf("%3d ) %2d @(%2d,%2d) z%4d\n", tstamp, tid, x, y, decimos);
}

void *thread_function(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int current_x, current_y;

    // Start at the first position
    current_x = data->positions[0].x;
    current_y = data->positions[0].y;

    pthread_mutex_lock(&board_mutexes[current_x][current_y]);
    passa_tempo(data->id, current_x, current_y, data->positions[0].time);
    pthread_mutex_unlock(&board_mutexes[current_x][current_y]);

    for (int i = 1; i < data->num_positions; i++) {
        int next_x = data->positions[i].x;
        int next_y = data->positions[i].y;

        // Try to move to the next position
        while (1) {
            pthread_mutex_lock(&board_mutexes[next_x][next_y]);
            // Check conditions to enter the next position
            // For simplicity, assume condition is met
            break;
        }

        // Move to the next position
        pthread_mutex_unlock(&board_mutexes[current_x][current_y]);
        passa_tempo(data->id, next_x, next_y, data->positions[i].time);
        current_x = next_x;
        current_y = next_y;
        pthread_mutex_unlock(&board_mutexes[next_x][next_y]);
    }

    return NULL;
}

int main() {
    scanf("%d %d", &N, &n_threads);
    pthread_t thread_ids[MAX_N];

    for (int i = 0; i < n_threads; i++) {
        scanf("%d %d %d", &threads[i].id, &threads[i].group_id, &threads[i].num_positions);
        threads[i].positions = malloc(threads[i].num_positions * sizeof(Position));

        for (int j = 0; j < threads[i].num_positions; j++) {
            scanf("%d %d %d", &threads[i].positions[j].x, &threads[i].positions[j].y, &threads[i].positions[j].time);
        }

        pthread_create(&thread_ids[i], NULL, thread_function, &threads[i]);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(thread_ids[i], NULL);
        free(threads[i].positions);
    }

    return 0;
}
