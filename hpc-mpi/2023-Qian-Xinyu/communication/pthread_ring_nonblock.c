#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#define nthread 4

struct ThreadData {
    int rank;
    int size;
    int thread_id;
};

void *send_receive(void *thread_data) 
{
    struct ThreadData *data = (struct ThreadData *)thread_data;
    int rank = data->rank;
    int size = data->size;
    int thread_id = data->thread_id;

    // Each thread has its own send and receive buffers
    int send_buf = rank * nthread + thread_id;
    int recv_buf;

    // non-blocking send and receive
    int source = (rank - 1 + size) % size;
    int dest = (rank + 1) % size;
    MPI_Request send_request, recv_request;

    MPI_Isend(&send_buf, 1, MPI_INT, dest, thread_id, MPI_COMM_WORLD, &send_request);

    MPI_Irecv(&recv_buf, 1, MPI_INT, source, thread_id, MPI_COMM_WORLD, &recv_request);

    // Wait for send and receive to complete
    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);

//    std::cout << "Rank " << rank << ", Thread " << thread_id << " received " << recv_buf << " from Rank " << source << ", Thread " << thread_id << std::endl;

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    double start, finish, elapsed_time, max_elapsed_time;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    if (provided != MPI_THREAD_MULTIPLE) {
        printf("MPI_THREAD_MULTIPLE not supported!");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start = MPI_Wtime();
    pthread_t threads[nthread];
    struct ThreadData thread_data[nthread];

    for (int i = 0; i < nthread; ++i) 
    {
        thread_data[i].rank = rank;
        thread_data[i].thread_id = i;
        thread_data[i].size = size;

        pthread_create(&threads[i], NULL, send_receive, (void *)&thread_data[i]);
    }

    for (int i = 0; i < nthread; ++i) {
        pthread_join(threads[i], NULL);
    }

    finish = MPI_Wtime();
    elapsed_time = finish - start;
    MPI_Reduce(&elapsed_time, &max_elapsed_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("Total communication time: %f seconds\n", max_elapsed_time);
    }

    MPI_Finalize();
    return 0;
}
