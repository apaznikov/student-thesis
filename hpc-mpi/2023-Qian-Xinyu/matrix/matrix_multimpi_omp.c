#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#define nthread 4

int main(int argc, char *argv[])
{
    int my_rank;
    int num_procs;
    int size = 3072;
    int element = 6;
    int i, j, k;
    double start, finish;

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (provided != MPI_THREAD_MULTIPLE)
    {
        printf("MPI_THREAD_MULTIPLE not provided!\n");
        MPI_Finalize();
        return 1;
    }

    int line = size / (num_procs-1);
    printf(" line = %d\n", line);
    int *local_a = (int *)malloc(line * size * sizeof(int));
    int *b = (int *)malloc(size * size * sizeof(int));
    int *ans = (int *)malloc(line * size * sizeof(int));
    int *a = (int *)malloc(size * size * sizeof(int));
    int *c = (int *)malloc(size * size * sizeof(int));
    MPI_Request req1[num_procs - 1], req2[num_procs - 1], req3[num_procs - 1];
    MPI_Status status1[num_procs - 1], status2[num_procs - 1], status3[num_procs - 1];

    if (my_rank == 0)
    {

        for (i = 0; i < size; i++)
        {
            for (j = 0; j < size; j++)
            {
                a[i * size + j] = element;
                b[i * size + j] = element + 2;
            }
        }

        start = MPI_Wtime();

#pragma omp parallel shared(a,b,c) private(i) num_threads(nthread)
{
// Scatter
        #pragma omp for schedule(static)
        for (i = 1; i < num_procs; i++)
        {
            MPI_Isend(a + (i-1) * line * size, line * size, MPI_INT, i, 0, MPI_COMM_WORLD, &req1[i-1]);
        }

// Bcast
        #pragma omp for schedule(static)
        for (i = 1; i < num_procs; i++)
        {
            MPI_Isend(b, size * size, MPI_INT, i, 1, MPI_COMM_WORLD, &req2[i-1]);
        }
        #pragma omp barrier

        #pragma omp single
        {
            MPI_Waitall(num_procs - 1, req1, status1);
        }
        #pragma omp single
        {
            MPI_Waitall(num_procs-1, req2, status2);
        }

        #pragma omp barrier

// Gather
        #pragma omp for schedule(static)
        for (i = 1; i < num_procs; i++)
        {
            MPI_Irecv(c + (i-1) * line * size, line * size, MPI_INT, i, 2, MPI_COMM_WORLD, &req3[i-1]);
        }
        
        #pragma omp single
        {
        MPI_Waitall(num_procs-1, req3, status3);
        }
}

        finish = MPI_Wtime();
        printf(" time: %lf s \n", finish - start);

        FILE *fp = fopen("C.txt", "w");
        for (i = 0; i < size; i++)
        {
            for (j = 0; j < size; j++)
                fprintf(fp, "%d\t", c[i * size + j]);
            fputc('\n', fp);
        }
        fclose(fp);
    }

    else
    {
#pragma omp parallel shared(a,b,ans) private(i,j,k) num_threads(nthread)
{
        #pragma omp single
        {
        MPI_Irecv(local_a, line * size, MPI_INT, 0, 0, MPI_COMM_WORLD, &req1[my_rank-1]);
        MPI_Wait(&req1[my_rank-1], MPI_STATUS_IGNORE);
        }

        #pragma omp single
        {
        MPI_Irecv(b, size * size, MPI_INT, 0, 1, MPI_COMM_WORLD, &req2[my_rank-1]);
        MPI_Wait(&req2[my_rank-1], MPI_STATUS_IGNORE);
        }

        #pragma omp barrier

        // Calculation
        #pragma omp for schedule(dynamic)
        for (i = 0; i < line; i++)
        {
            for (j = 0; j < size; j++)
            {
                int temp = 0;
                for (k = 0; k < size; k++)
                    temp += local_a[i * size + k] * b[k * size + j];
                ans[i * size + j] = temp;
            }
        }

        #pragma omp barrier
        // Gather
        #pragma omp single
        {
        MPI_Isend(ans, line * size, MPI_INT, 0, 2, MPI_COMM_WORLD, &req3[my_rank-1]);
        MPI_Wait(&req3[my_rank-1], MPI_STATUS_IGNORE);
        }
}
    }

    free(a);
    free(local_a);
    free(b);
    free(ans);
    free(c);

    MPI_Finalize();
    return 0;
}
