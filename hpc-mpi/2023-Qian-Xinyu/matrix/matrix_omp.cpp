#include <iostream>
#include <chrono>
#include <cstdio>
#include <omp.h>
#define nthread 8
using namespace std;

int main(int argc, char *argv[])
{
    int size = 1440;
    int element = 6;
    int* a = new int[size * size];
    int* b = new int[size * size];
    int* c = new int[size * size];
    int i,j,k;

    for (i = 0; i < size; i++) 
    {
        for (j = 0; j < size; j++) 
        {
            a[i * size + j] = element;
            b[i * size + j] = element + 2;
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    omp_set_num_threads(nthread);
#pragma omp parallel shared(a,b,c) private(i,j,k)
{
    #pragma omp for schedule(dynamic)
    for (i = 0; i < size; i++) 
    {
        for (j = 0; j < size; j++) 
        {
            int temp = 0;
            for (k = 0; k < size; k++)
            {
                temp += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = temp;
        }
    }
}

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration<double>(end_time - start_time).count();
    std::cout << "Elapsed time: " << elapsed_time << " s" << std::endl;

    FILE* fp = fopen("C.txt", "w");
    for (i = 0; i < size; i++) 
    {
        for (j = 0; j < size; j++)
            fprintf(fp, "%d\t", c[i * size + j]);
        fputc('\n', fp);
    }
    fclose(fp);

    delete[] a, b, c;

    return 0;
}
