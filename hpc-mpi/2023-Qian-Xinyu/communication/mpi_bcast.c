#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rank, size, number;
    double start, finish;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) 
    {
        int *value = (int *)malloc(3000 * 3000 * sizeof(int));

        start = MPI_Wtime();
        MPI_Bcast(value, 3000 * 3000, MPI_INT, 0, MPI_COMM_WORLD);

        finish = MPI_Wtime();

        printf(" time: %lf s \n", finish - start);
        free(value);
    }
    else
    {
        int *buf = (int *)malloc(3000 * 3000 * sizeof(int));
        MPI_Bcast(buf, 3000 * 3000, MPI_INT, 0, MPI_COMM_WORLD);
        free(buf);
    }

//    std::cout << "Process " << rank << ": The number is " << number << std::endl;
    MPI_Finalize();
    return 0;
}

