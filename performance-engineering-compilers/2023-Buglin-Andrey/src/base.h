#pragma once
#ifndef _BASE_H
#define _BASE_H

#include <string>
#include <stdint.h>
#include <emmintrin.h>
#include <xmmintrin.h>

using namespace std;

namespace Base
{
    class OutputFunctions; // Класс для вывода результатов, ошибок и оценки производительности
    class Configuration; // Класс для конфигурации бенчмарка
    class Benchmark; // Класс для выполнения бенчмарка
    class Benchmarks; // Класс для хранения и выполнения всех бенчмарков

    typedef void     (*PrintFunction)(char* str); // Тип функции для вывода текста
    typedef bool     (*InitFunction)(void); // Тип функции инициализации
    typedef bool     (*CleanupFunction)(void); // Тип функции очистки
    typedef uint64_t(*KernelFunction)(uint64_t n); // Тип функции ядра бенчмарка

    template<typename X4, typename X>
    class Lanes
    {
    private:
        union
        {
            X4  m128;
            X   lanes[4];
        } lanes;
    public:
        Lanes(X4 m128)
        {
            lanes.m128 = m128;
        }
        X x()
        {
            return lanes.lanes[0];
        }
        X y()
        {
            return lanes.lanes[1];
        }
        X z()
        {
            return lanes.lanes[2];
        }
        X w()
        {
            return lanes.lanes[3];
        }
    };

    typedef union
    {
        float  m128_f32[4];
        __m128 f32x4;
    } M128;

#define M128_INIT(m128) Base::M128 m128##overlay; m128##overlay.f32x4 = m128
#define M128_X(m128) (m128##overlay.m128_f32[0])
#define M128_Y(m128) (m128##overlay.m128_f32[1])
#define M128_Z(m128) (m128##overlay.m128_f32[2])
#define M128_W(m128) (m128##overlay.m128_f32[3])

    typedef union
    {
        int  m128i_i32[4];
        __m128i i32x4;
    } M128I;

#define M128I_INIT(m128i) Base::M128I m128i##overlay; m128i##overlay.i32x4 = m128i
#define M128I_X(m128i) (m128i##overlay.m128i_i32[0])
#define M128I_Y(m128i) (m128i##overlay.m128i_i32[1])
#define M128I_Z(m128i) (m128i##overlay.m128i_i32[2])
#define M128I_W(m128i) (m128i##overlay.m128i_i32[3])

    class OutputFunctions
    {
    public:
        OutputFunctions(PrintFunction PrintResult, PrintFunction PrintError, PrintFunction PrintScore)
            : PrintResult(PrintResult)
            , PrintError(PrintError)
            , PrintScore(PrintScore)
        {
        }

        PrintFunction PrintResult; // Функция для вывода результатов
        PrintFunction PrintError; // Функция для вывода ошибок
        PrintFunction PrintScore; // Функция для вывода оценки производительности
    };

    class Configuration
    {
    public:
        Configuration(string        name,
            InitFunction    Init,
            CleanupFunction Cleanup,
            KernelFunction  Simd,
            KernelFunction  nonSimd32,
            KernelFunction  nonSimd64,
            uint64_t        iterations)
            : kernelName(name)
            , kernelInit(Init)
            , kernelCleanup(Cleanup)
            , kernelSimd(Simd)
            , kernelNonSimd32(nonSimd32)
            , kernelNonSimd64(nonSimd64)
            , kernelIterations(iterations)
        {
        }

        string           kernelName; // Название бенчмарка
        InitFunction     kernelInit; // Функция инициализации
        CleanupFunction  kernelCleanup; // Функция очистки
        KernelFunction   kernelSimd; // Функция ядра бенчмарка для SIMD
        KernelFunction   kernelNonSimd32; // Функция ядра бенчмарка для 32-битной архитектуры
        KernelFunction   kernelNonSimd64; // Функция ядра бенчмарка для 64-битной архитектуры
        uint64_t         kernelIterations; // Количество итераций бенчмарка
    };

    class Benchmarks
    {
    public:
        static void RunAll(OutputFunctions& outputFunctions, bool useAutoIterations); // Выполнение всех бенчмарков
        static void Add(Benchmark* benchmark); // Добавление бенчмарка
    };

    extern Benchmarks benchmarks; // Объект для хранения и выполнения всех бенчмарков

    class Benchmark
    {
    public:
        Benchmark(Configuration* config)
            : config(config)
            , useAutoIterations(false)
            , initOk(true)
            , cleanupOk(true)
        {
            Base::benchmarks.Add(this);
        }

        Configuration* config; // Конфигурация бенчмарка
        bool           useAutoIterations; // Флаг использования автоматического количества итераций
        bool           initOk; // Флаг успешной инициализации
        bool           cleanupOk; // Флаг успешной очистки
        uint64_t       autoIterations; // Автоматическое количество итераций
        uint64_t       actualIterations; // Фактическое количество итераций
        uint64_t       simdTime; // Время выполнения SIMD-алгоритма
        uint64_t       nonSimd32Time; // Время выполнения алгоритма для 32-битной архитектуры
        uint64_t       nonSimd64Time; // Время выполнения алгоритма для 64-битной архитектуры
    };
} //namespace Base
#endif