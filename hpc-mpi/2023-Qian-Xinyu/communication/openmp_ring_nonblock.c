#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    int provided, required = MPI_THREAD_MULTIPLE;
    double start, finish, elapsed_time, max_elapsed_time;

    // Initialize the MPI environment
    MPI_Init_thread(&argc, &argv, required, &provided);

    // Check if MPI_THREAD_MULTIPLE is supported
    if (provided != required) {
        fprintf(stderr, "Error: MPI implementation does not support MPI_THREAD_MULTIPLE");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Set the number of OpenMP threads
    omp_set_num_threads(4);

    start = MPI_Wtime();
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        // Each thread has its own send and receive buffers
        int send_buf = rank * num_threads + thread_id;
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

//        #pragma omp critical
//        {
//            std::cout << "Rank " << rank << ", Thread " << thread_id << " received " << recv_buf << " from Rank " << source << ", Thread " << thread_id << std::endl;
//        }
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
