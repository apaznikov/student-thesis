#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <list>
#include <sys/types.h>
#include <sys/timeb.h>
#include "base.h"

using namespace std;

typedef list<Base::Benchmark *> BenchmarkList;

static BenchmarkList benchmarkList;

static uint64_t Now() 
{
    struct timeb time;
    ftime(&time);
    return time.time * 1000 + time.millitm;
}

static uint64_t TimeKernel(Base::KernelFunction kernel, uint64_t iterations)
{
    uint64_t start, stop;
    start = Now();
    kernel(iterations);
    stop = Now();
    return stop - start;
}

static uint64_t ComputeIterations(Base::Benchmark *benchmark)
{
    uint64_t desiredRuntime = 1000;  // миллисекунды для самого долго работающего ядра
    uint64_t testIterations = 10;    // итерации, используемые для определения времени для desiredRuntime

    // Заставить самое медленное ядро работать не менее 500 мс
    uint64_t simdTime = TimeKernel(benchmark->config->kernelSimd, testIterations);
    uint64_t nonSimdTime = TimeKernel(benchmark->config->kernelNonSimd32, testIterations);
    uint64_t maxTime = simdTime > nonSimdTime ? simdTime : nonSimdTime;
    while (maxTime < 500)
    {
        testIterations *= 2;
        simdTime = TimeKernel(benchmark->config->kernelSimd, testIterations);
        nonSimdTime = TimeKernel(benchmark->config->kernelNonSimd32, testIterations);
        maxTime = simdTime > nonSimdTime ? simdTime : nonSimdTime;
        //    printf("testIterations: %llu, maxTime: %llu\n", testIterations, maxTime);
    }
    maxTime = simdTime > nonSimdTime ? simdTime : nonSimdTime;

    // Вычислить количество итераций для 1-секундного запуска самого медленного ядра
    uint64_t iterations = desiredRuntime * testIterations / maxTime;
    return iterations;
}

static bool RunOne(Base::Benchmark *benchmark)
{
    // Инициализировать ядра и проверить состояние корректности
    if (!benchmark->config->kernelInit())
    {
        benchmark->initOk = false;
        return false;
    }

    // Определить, сколько итераций использовать
    if (benchmark->useAutoIterations) 
    {
        benchmark->autoIterations = ComputeIterations(benchmark);
        benchmark->actualIterations = benchmark->autoIterations;
    }
    else 
    {
        benchmark->actualIterations = benchmark->config->kernelIterations;
    }

    // Run the SIMD kernel
    benchmark->simdTime = TimeKernel(benchmark->config->kernelSimd, benchmark->actualIterations);

    // Run the non-SIMD kernels
    benchmark->nonSimd32Time = TimeKernel(benchmark->config->kernelNonSimd32, benchmark->actualIterations);
    benchmark->nonSimd64Time = TimeKernel(benchmark->config->kernelNonSimd64, benchmark->actualIterations);

    // Do the final sanity check
    if (!benchmark->config->kernelCleanup())
    {
        benchmark->cleanupOk = false;
        return false;
    }
    return true;
}

static void PrintHeaders(Base::PrintFunction printFunction)
{
    char buf[200];
    sprintf(buf, "%-20s : %12s %12s %12s %12s %10s %10s",
        "Name", "Iterations", "Scalar32(ns)", "Scalar64(ns)", "SIMD32(ns)", "Ratio32", "Ratio64");
    printFunction(buf);
}

static void PrintColumns(Base::PrintFunction printFunction, const char *name, uint64_t iterations, uint64_t scalar32, uint64_t scalar64, uint64_t simd32, double ratio32, double ratio64)
{
    char buf[200];
    sprintf(buf, "%-20s : %12llu %12llu %12llu %12llu %10.2f %10.2f",
        name, iterations, scalar32, scalar64, simd32, ratio32, ratio64);
    printFunction(buf);
}

static void Report(Base::Benchmark *benchmark, Base::OutputFunctions &outputFunctions) 
{
    char buf[200];
    if (!benchmark->initOk) 
    {
        sprintf(buf, "%s: %s", benchmark->config->kernelName.c_str(), "FAILED INIT");
        outputFunctions.PrintError(buf);
        return;
    }
    if (!benchmark->cleanupOk)
    {
        sprintf(buf, "%s: %s", benchmark->config->kernelName.c_str(), "FAILED CLEANUP");
        outputFunctions.PrintError(buf);
        return;
    }
    double ratio32 = (double)benchmark->nonSimd32Time / (double)benchmark->simdTime;
    double ratio64 = (double)benchmark->nonSimd64Time / (double)benchmark->simdTime;
    PrintColumns(
        outputFunctions.PrintResult,
        benchmark->config->kernelName.c_str(),
        benchmark->actualIterations,
        benchmark->nonSimd32Time * 1000 * 1000 / benchmark->actualIterations,
        benchmark->nonSimd64Time * 1000 * 1000 / benchmark->actualIterations,
        benchmark->simdTime * 1000 * 1000 / benchmark->actualIterations,
        ratio32,
        ratio64);
}

void Base::Benchmarks::RunAll(Base::OutputFunctions &outputFunctions, bool useAutoIterations)
{
    //  printf("RunAll\n");
    PrintHeaders(outputFunctions.PrintResult);
    for (BenchmarkList::iterator it = benchmarkList.begin(); it != benchmarkList.end(); it++) 
    {
        Base::Benchmark* benchmark = *it;
        //    printf("Running: %s\n", benchmark->config->kernelName.c_str());
        benchmark->useAutoIterations = useAutoIterations;
        RunOne(benchmark);
        Report(benchmark, outputFunctions);
    }
}

void Base::Benchmarks::Add(Base::Benchmark *benchmark) 
{
    //  printf("adding: %s\n", benchmark->config->kernelName.c_str());
    benchmarkList.push_back(benchmark);
}