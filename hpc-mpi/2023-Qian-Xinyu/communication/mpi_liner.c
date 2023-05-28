#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    double start, finish;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int *value = (int *)malloc(3000 * 3000 * sizeof(int));
        start = MPI_Wtime();

        for (int i = 1; i < size; i++) {
            MPI_Send(value, 3000 * 3000, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        finish = MPI_Wtime();

        printf(" time: %lf s \n", finish - start);
        printf("Process %d sent %p to all processes\n", rank, (void *)value);

        free(value);
    } else {
        int *buf = (int *)malloc(3000 * 3000 * sizeof(int));
        MPI_Recv(buf, 3000 * 3000, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        free(buf);
    }

    MPI_Finalize();
    return 0;
}
