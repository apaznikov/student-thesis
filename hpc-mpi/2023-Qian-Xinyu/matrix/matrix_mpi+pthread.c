#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#define nthread 4

struct thread_data
{
    int id;
    int line;
    int size;
    int *local_a;
    int *b;
    int *ans;
};

void *matrix_multiply(void *data)
{
    struct thread_data *td = (struct thread_data *)data;
    int id = td->id;
    int line = td->line;
    int size = td->size;
    int *local_a = td->local_a;
    int *b = td->b;
    int *ans = td->ans;

    for (int i = id * line; i < (id + 1) * line; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int temp = 0;
            for (int k = 0; k < size; k++)
            {
                temp += local_a[i * size + k] * b[k * size + j];
            }
            ans[i * size + j] = temp;
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int my_rank;
    int num_procs;
    int size = 1440;
    int element = 6;
    int i, j, k;
    double start, finish;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int line = size / num_procs;
    printf(" line = %d\n", line);
    int *local_a = (int *)malloc(line * size * sizeof(int));
    int *b = (int *)malloc(size * size * sizeof(int));
    int *ans = (int *)malloc(line * size * sizeof(int));
    int *a = (int *)malloc(size * size * sizeof(int));
    int *c = (int *)malloc(size * size * sizeof(int));

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

        MPI_Scatter(a, line * size, MPI_INT, local_a, line * size, MPI_INT, 0, MPI_COMM_WORLD);

        MPI_Bcast(b, size * size, MPI_INT, 0, MPI_COMM_WORLD);

        pthread_t threads[nthread];
        struct thread_data td[nthread];
        int local_line = line / nthread;

        for (i = 0; i < nthread; i++)
        {
            td[i].id = i;
            td[i].line = local_line;
            td[i].size = size;
            td[i].local_a = local_a;
            td[i].b = b;
            td[i].ans = ans;
            pthread_create(&threads[i], NULL, matrix_multiply, (void *)&td[i]);
        }

        for (i = 0; i < nthread; i++)
        {
            pthread_join(threads[i], NULL);
        }

        MPI_Gather(ans, line * size, MPI_INT, c, line * size, MPI_INT, 0, MPI_COMM_WORLD);

        for (i = num_procs * line; i < size; i++) 
        {
            for (j = 0; j < size; j++) {
                int temp = 0;
                for (k = 0; k < size; k++)
                    temp += a[i * size + k] * b[k * size + j];
                c[i * size + j] = temp;
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
        int *buffer = (int *)malloc(size * line * sizeof(int));
        MPI_Scatter(a, line * size, MPI_INT, buffer, line * size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(b, size * size, MPI_INT, 0, MPI_COMM_WORLD);

        pthread_t threads[nthread];
        struct thread_data td[nthread];
        int local_line = line / nthread;

        for (i = 0; i < nthread; i++)
        {
            td[i].id = i;
            td[i].line = local_line;
            td[i].size = size;
            td[i].local_a = buffer;
            td[i].b = b;
            td[i].ans = ans;
            pthread_create(&threads[i], NULL, matrix_multiply, (void *)&td[i]);
        }

        for (i = 0; i < nthread; i++)
        {
            pthread_join(threads[i], NULL);
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
