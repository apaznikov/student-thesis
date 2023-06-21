#pragma once
#ifndef _AVERAGEFLOAT32X4_H
#define _AVERAGEFLOAT32X4_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "base.h"

class AverageFloat32x4 : public Base::Benchmark
{
public:
    AverageFloat32x4()
        : Base::Benchmark(
            new Base::Configuration(
                string("AverageFloat32x4"), // Название бенчмарка
                InitArray, // Функция инициализации массива
                Cleanup, // Функция очистки
                SimdAverage, // Функция ядра SIMD
                Average32, // Функция ядра для 32-битной архитектуры
                Average64, // Функция ядра для 64-битной архитектуры
                1000)) {} // Количество итераций

    static uint64_t preventOptimize;

    static const uint32_t length = 10000; // Длина массива
    static float a[length];

    static bool SanityCheck()
    {
        float simdVal = SimdAverageKernel();
        float nonSimd32Val = NonSimdAverageKernel32();
        float nonSimd64Val = (float)NonSimdAverageKernel64();
        return fabs(simdVal - nonSimd32Val) < 0.0001 &&
            fabs(simdVal - nonSimd64Val) < 0.0001;
    }

    static bool InitArray()
    {
        for (uint32_t i = 0; i < length; ++i)
        {
            a[i] = 0.1f;
        }
        // Проверка, что две функции ядра дают одинаковый результат.
        return SanityCheck();
    }

    static bool Cleanup()
    {
        return SanityCheck();
    };

    static float SimdAverageKernel()
    {
        preventOptimize++;
        __m128 sumx4 = _mm_set_ps1(0.0);
        for (uint32_t j = 0, l = length; j < l; j = j + 4)
        {
            sumx4 = _mm_add_ps(sumx4, _mm_loadu_ps(&(a[j])));
        }
        Base::Lanes<__m128, float> lanes(sumx4);
        return (lanes.x() + lanes.y() + lanes.z() + lanes.w()) / length;
    }

    static float NonSimdAverageKernel32()
    {
        preventOptimize++;
        float sum = 0.0;
        for (uint32_t j = 0, l = length; j < l; ++j)
        {
            sum += a[j];
        }
        return sum / length;
    }

    static double NonSimdAverageKernel64()
    {
        preventOptimize++;
        double sum = 0.0;
        for (uint32_t j = 0, l = length; j < l; ++j)
        {
            sum += (double)a[j];
        }
        return sum / length;
    }

    static uint64_t SimdAverage(uint64_t n)
    {
        float val;
        for (uint64_t i = 0; i < n; ++i)
        {
            val = SimdAverageKernel();
        }
        return (uint64_t)val;
    };

    static uint64_t Average32(uint64_t n)
    {
        float val;
        for (uint64_t i = 0; i < n; ++i)
        {
            val = NonSimdAverageKernel32();
        }
        return (uint64_t)val;
    };

    static uint64_t Average64(uint64_t n)
    {
        double val;
        for (uint64_t i = 0; i < n; ++i)
        {
            val = NonSimdAverageKernel64();
        }
        return (uint64_t)val;
    };
};

uint64_t AverageFloat32x4::preventOptimize = 0;
float AverageFloat32x4::a[AverageFloat32x4::length];

#endif