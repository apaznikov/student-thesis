#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#define nthread 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int work_done = 0;

struct Args
{
    int line;
    int size;
    int my_rank;
    int num_procs;
    int *a;
    int *b;
    int *local_a;
    int *ans;
    int *c;
};

void *scatter_thread(void *args_ptr)
{
    struct Args *args = (struct Args *)args_ptr;
    int line = args->line;
    int size = args->size;
    int my_rank = args->my_rank;
    int num_procs = args->num_procs;
    int *a = args->a;
    int *local_a = args->local_a;

    MPI_Request request1[num_procs];
    MPI_Status status1[num_procs];

    if (my_rank == 0)
    {
        for (int p = 1; p < num_procs; ++p)
        {
            MPI_Isend(a + p * line * size, line * size, MPI_INT, p, 0, MPI_COMM_WORLD, &request1[p - 1]);
        }
        MPI_Waitall(num_procs-1, request1, status1);
    }
    else
    {
        MPI_Irecv(local_a, line * size, MPI_INT, 0, 0, MPI_COMM_WORLD, &request1[my_rank - 1]);
        MPI_Wait(&request1[my_rank - 1], MPI_STATUS_IGNORE);
    }
    pthread_exit(NULL);
}

void *bcast_thread(void *args_ptr)
{
    struct Args *args = (struct Args *)args_ptr;
    int size = args->size;
    int *b = args->b;

    MPI_Request request;
    MPI_Ibcast(b, size * size, MPI_INT, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);

    pthread_exit(NULL);
}

void *gather_thread(void *args_ptr)
{
    struct Args *args = (struct Args *)args_ptr;
    int line = args->line;
    int size = args->size;
    int my_rank = args->my_rank;
    int num_procs = args->num_procs;
    int *ans = args->ans;
    int *c = args->c;

    MPI_Request request2[num_procs];
    MPI_Status status2[num_procs];

    pthread_mutex_lock(&mutex);
    while (!work_done)
        pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    if (my_rank == 0)
    {
        for (int p = 1; p < num_procs; ++p)
        {
            MPI_Irecv(c + p * line * size, line * size, MPI_INT, p, 1, MPI_COMM_WORLD, &request2[p - 1]);
        }
        MPI_Waitall(num_procs - 1, request2, status2);
    }
    else
    {
        MPI_Isend(ans, line * size, MPI_INT, 0, 1, MPI_COMM_WORLD, &request2[my_rank - 1]);
        MPI_Wait(&request2[my_rank - 1], MPI_STATUS_IGNORE);
    }
    pthread_exit(NULL);
}

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
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);   // Get the current process number
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // Get the number of processes

    int line = size / num_procs; // Divide the data into (number of processes) blocks
    printf(" line = %d\n", line);
    int *local_a = (int *)malloc(line * size * sizeof(int));
    int *b = (int *)malloc(size * size * sizeof(int));
    int *ans = (int *)malloc(line * size * sizeof(int));
    int *a = (int *)malloc(size * size * sizeof(int));
    int *c = (int *)malloc(size * size * sizeof(int));

    if (provided < MPI_THREAD_MULTIPLE)
    {
        printf("MPI_THREAD_MULTIPLE not supported!");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (my_rank == 0)
    {
        for (i = 0; i < size; i++)
        {
            for (j = 0; j < size; j++)
            {
                a[i * size + j] = element;
                b[i * size + j] = element + 2;
            }
        } // Create Matrix a and b
    }

    start = MPI_Wtime(); 
    pthread_t scatter, bcast, gather;
    struct Args args = {line, size, my_rank, num_procs, a, b, local_a, ans, c};
    pthread_create(&scatter, NULL, scatter_thread, (void *)&args);
    pthread_create(&bcast, NULL, bcast_thread, (void *)&args);
    pthread_create(&gather, NULL, gather_thread, (void *)&args);
    pthread_join(scatter, NULL);
    pthread_join(bcast, NULL);

    if (my_rank == 0)
    {
#pragma omp parallel shared(a, b, c) private(i, j, k) num_threads(nthread)
        {
            #pragma omp for schedule(dynamic)
            for (i = 0; i < line; i++)
            {
                for (j = 0; j < size; j++)
                {
                    int temp = 0;
                    for (k = 0; k < size; k++)
                        temp += a[i * size + k] * b[k * size + j];
                    c[i * size + j] = temp;
                }
            }
#pragma omp barrier
// Process remaining lines if size is not evenly divisible by num_procs
            #pragma omp for schedule(dynamic)
            for (int i = num_procs * line; i < size; i++)
            {
                for (int j = 0; j < size; j++)
                {
                    int temp = 0;
                    for (int k = 0; k < size; k++)
                    {
                        temp += a[i * size + k] * b[k * size + j];
                    }
                    c[i * size + j] = temp;
                }
            }
        }

        pthread_mutex_lock(&mutex);
        work_done = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        pthread_join(gather, NULL);

        finish = MPI_Wtime();
        printf(" time: %lf s \n", finish - start);

        FILE *fp = fopen("C.txt", "w");
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                fprintf(fp, "%d\t", c[i * size + j]);
            }
            fputc('\n', fp);
        }
        fclose(fp);
    }
    else
    {
#pragma omp parallel shared(local_a, b, ans) private(i, j, k) num_threads(nthread)
{
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
}
        pthread_mutex_lock(&mutex);
        work_done = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        pthread_join(gather, NULL);
    }

    free(a);
    free(local_a);
    free(b);
    free(ans);
    free(c);

    MPI_Finalize();
    return 0;
}

