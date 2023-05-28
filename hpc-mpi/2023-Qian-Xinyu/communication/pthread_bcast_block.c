#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREAD 60

typedef struct thread_data {
    int *value;
    int rank;
    int size;
    MPI_Request *request;
} thread_data;

void *sender_thread_func(void *arg) {
    thread_data *tdata = (thread_data *)arg;
    int tid = tdata->rank;
    int size = tdata->size;
    int *value = tdata->value;

    MPI_Send(value, 3000 * 3000, MPI_INT, tid, 0, MPI_COMM_WORLD);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int rank, size;
    double start, finish;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int *value = (int *)malloc(3000 * 3000 * sizeof(int));
        start = MPI_Wtime();

        pthread_t threads[NTHREAD];
        MPI_Request *request = malloc((size - 1) * sizeof(MPI_Request));
        thread_data *tdata = malloc((size - 1) * sizeof(thread_data));

        for (int i = 1; i < size; i++) {
            tdata[i - 1].value = value;
            tdata[i - 1].rank = i;
            tdata[i - 1].size = size;
            tdata[i - 1].request = request;
            pthread_create(&threads[i - 1], NULL, sender_thread_func, (void *)&tdata[i - 1]);
        }

        for (int i = 0; i < size - 1; i++) {
            pthread_join(threads[i], NULL);
        }

        finish = MPI_Wtime();

        printf(" time: %lf s \n", finish - start);
        printf("Process %d sent %p to all processes\n", rank, (void *)value);

        free(value);
        free(request);
        free(tdata);
    } else {
        int *buf = (int *)malloc(3000 * 3000 * sizeof(int));
        MPI_Recv(buf, 3000 * 3000, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        free(buf);
    }

    MPI_Finalize();
    return 0;
}
