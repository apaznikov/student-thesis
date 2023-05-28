#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

    int my_rank;
    int num_procs;
    int size = 1440;
    int element = 6;
    int i,j,k;
    double start, finish;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //Get the current process number
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); //Get the number of processes

    int line = size / num_procs; //Divide the data into (number of processes) blocks
    printf(" line = %d\n", line);
    int *local_a = (int *)malloc(line * size * sizeof(int));
    int *b = (int *)malloc(size * size * sizeof(int));
    int *ans = (int *)malloc(line * size * sizeof(int));
    int *a = (int *)malloc(size * size * sizeof(int));
    int *c = (int *)malloc(size * size * sizeof(int));

    if (my_rank == 0) {

        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                a[i * size + j] = element;
                b[i * size + j] = element + 2;
            }
        } //Create Matrix a and b

        start = MPI_Wtime();

        MPI_Scatter(a, line * size, MPI_INT, local_a, line * size, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Bcast(b, size * size, MPI_INT, 0, MPI_COMM_WORLD);

        for (i = 0; i < line; i++) {
            for (j = 0; j < size; j++) {
                int temp = 0;
                for (k = 0; k < size; k++)
                    temp += a[i * size + k] * b[k * size + j];
                ans[i * size + j] = temp;
            }
        }

        MPI_Gather(ans, line * size, MPI_INT, c, line * size, MPI_INT, 0, MPI_COMM_WORLD);

        for (i = num_procs * line; i < size; i++) {
            for (j = 0; j < size; j++) {
                int temp = 0;
                for (k = 0; k < size; k++)
                    temp += a[i * size + k] * b[k * size + j];
                c[i * size + j] = temp;
            }
        }

        finish = MPI_Wtime();
        printf(" time: %lf s \n", finish - start);

        FILE* fp = fopen("C.txt", "w");
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++)
                fprintf(fp, "%d\t", c[i * size + j]);
            fputc('\n', fp);
        }
        fclose(fp);

    }

    else {
        int* buffer = (int*) malloc(size * line * sizeof(int));
        MPI_Scatter(a, line * size, MPI_INT, buffer, line * size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(b, size * size, MPI_INT, 0, MPI_COMM_WORLD);
        

        for (i = 0; i < line; i++) {
            for (j = 0; j < size; j++) {
                int temp = 0;
                for (k = 0; k < size; k++)
                    temp += buffer[i * size + k] * b[k * size + j];
                ans[i * size + j] = temp;
            }
        }

        MPI_Gather(ans, line * size, MPI_INT, c, line * size, MPI_INT, 0, MPI_COMM_WORLD);
        free(buffer);
    }

    free(a);
    free(local_a);
    free(b);
    free(ans);
    free(c);

    MPI_Finalize();
    return 0;
}
