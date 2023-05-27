#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define nthread 4

int main(int argc, char *argv[]) 
{
    int i, rank, size;
    int provided;
    double start, finish;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) 
    {
        int* value = (int*) malloc(3000 * 3000 * sizeof(int));
        start = MPI_Wtime();
#pragma omp parallel private(i) num_threads(nthread)
{
        #pragma omp for schedule(static)
        for (i = 1; i < size; i++) 
        {
            MPI_Send(value, 3000*3000, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
}
        finish = MPI_Wtime();
        printf("Process %d sent %p to all processes\n", rank, (void *)value);
        printf(" time: %lf s \n", finish - start);
        free(value);
    } 
    else 
    {
        int* buf = (int*) malloc(3000 * 3000 * sizeof(int));
        MPI_Recv(buf, 3000*3000, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //cout << "Process " << rank << " received " << value << endl;
        free(buf);
    }
    MPI_Finalize();
    return 0;
}

