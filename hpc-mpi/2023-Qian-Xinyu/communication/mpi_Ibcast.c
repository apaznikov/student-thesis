#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rank, size, number;
    MPI_Request request;
    double start, finish;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* value = (int*) malloc(3000 * 3000 * sizeof(int));

    start = MPI_Wtime();
    MPI_Ibcast(value, 3000*3000, MPI_INT, 0, MPI_COMM_WORLD, &request);

    MPI_Wait(&request, MPI_STATUS_IGNORE);

    finish = MPI_Wtime();

    if (rank == 0)
    {
        printf(" time: %lf s \n", finish - start);
    }

//    std::cout << "Process " << rank << ": The number is " << number << std::endl;
    free(value);
    MPI_Finalize();
    return 0;
}
