#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 20

// Struct Position: armazena uma posição e o tempo que será gasto nela
typedef struct {
    int x, y, time;
} Position;

// Struct ThreadData: armazena informações sobre a thread
typedef struct {
    int id;
    int group_id;
    int num_positions;
    Position *positions;
} ThreadData;

// Struct Cell: armazena informações sobre quem está numa célula, para
// podermos saber quando uma thread pode acessá-la ou não. Cada célula
// terá uma variável de condição.
typedef struct {
    int group_id;
    pthread_cond_t cond;
} Cell;

int N, n_threads;
ThreadData threads[MAX_N]; // Vetor de threads
pthread_mutex_t board_mutexes[MAX_N][MAX_N]; // Matriz de mutexes, um para cada célula
Cell board[MAX_N][MAX_N]; // Matriz de células

/*
Cada célula da matriz board tem um mutex associado a ela, armazenado na matriz 
board_mutexes. Esses mutexes são usados para garantir que apenas uma thread possa 
acessar ou modificar uma célula por vez
*/

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

    // Guarda a posição inicial da thread
    int current_x = data->positions[0].x;
    int current_y = data->positions[0].y;

    // Loop pelas posições que a thread vai percorrer
    for (int i = 0; i < data->num_positions; i++) {

        // Pega a próxima célula
        int next_x = data->positions[i].x;
        int next_y = data->positions[i].y;

        /*
        Antes de uma thread acessar uma célula (para verificar se está livre ou para ocupar), 
        ela deve bloquear o mutex correspondente usando pthread_mutex_lock.
        */
        pthread_mutex_lock(&board_mutexes[next_x][next_y]);

        /*
        Se uma thread tenta acessar uma célula que já está ocupada por uma thread do mesmo grupo, 
        ela espera na variável de condição associada à célula usando pthread_cond_wait.
        */
        while (board[next_x][next_y].group_id != -1 && board[next_x][next_y].group_id == data->group_id) {
            pthread_cond_wait(&board[next_x][next_y].cond, &board_mutexes[next_x][next_y]);
        }

        // Liberação da célula anterior - apenas se não for a primeira iteração
        /*
        Depois que uma thread termina de usar uma célula, ela deve liberar a célula para que outras threads 
        possam acessá-la. Isso é feito definindo group_id para -1 e chamando pthread_cond_broadcast para sinalizar 
        todas as threads que estão esperando na variável de condição associada à célula.
        */
        if (i > 0) {
            pthread_mutex_lock(&board_mutexes[current_x][current_y]);   // Bloqueia o mutex para fazer alterações
            board[current_x][current_y].group_id = -1;                  // Libera a célula anterior - (group_id = -1)
            pthread_cond_broadcast(&board[current_x][current_y].cond);  // Notifica outras threads que a célula está livre agora
            pthread_mutex_unlock(&board_mutexes[current_x][current_y]); // Desbloqueia o mutex
        }

        // Atualiza a célula com o grupo que está ocupando ela
        board[next_x][next_y].group_id = data->group_id;

        // Libera o mutex da célula em que a thread entrou
        pthread_mutex_unlock(&board_mutexes[next_x][next_y]);
        
        // Atualiza a célula atual onde está a thread
        current_x = next_x;
        current_y = next_y;

        // Executa a passa_tempo
        passa_tempo(data->id, current_x, current_y, data->positions[i].time);
    }

    // Libera a última célula quando a thread termina sua rota
    pthread_mutex_lock(&board_mutexes[current_x][current_y]);
    board[current_x][current_y].group_id = -1;
    pthread_cond_broadcast(&board[current_x][current_y].cond);
    pthread_mutex_unlock(&board_mutexes[current_x][current_y]);

    return NULL;
}

int main() {
    scanf("%d %d", &N, &n_threads);
    pthread_t thread_ids[MAX_N];

    // Obtém as informações das threads via input
    for (int i = 0; i < n_threads; i++) {
        scanf("%d %d %d", &threads[i].id, &threads[i].group_id, &threads[i].num_positions);
        threads[i].positions = malloc(threads[i].num_positions * sizeof(Position));

        for (int j = 0; j < threads[i].num_positions; j++) {
            scanf("%d %d %d", &threads[i].positions[j].x, &threads[i].positions[j].y, &threads[i].positions[j].time);
        }

    }

    // Cria as threads
    for (int i = 0; i < n_threads; i++) {
        pthread_create(&thread_ids[i], NULL, thread_function, &threads[i]);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(thread_ids[i], NULL);
        free(threads[i].positions);
    }

    return 0;
}