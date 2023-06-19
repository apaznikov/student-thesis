#pragma once
#ifndef _MATRIX_MULTIPLICATION_H
#define _MATRIX_MULTIPLICATION_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "base.h"

class MatrixMultiplication : public Base::Benchmark 
{
public:
    MatrixMultiplication() 
        : Base::Benchmark(
            new Base::Configuration(
                string("MatrixMultiplication"),
                Init,
                Cleanup,
                SimdMultiply,
                Multiply32,
                Multiply64,
                1000)) {}

    static uint64_t preventOptimize;

    static float* t1;
    static float* t2;
    static float* out;
    static float* t1x4;
    static float* t2x4;
    static float* outx4;

    static bool Equals(const float* t1, const float* t2) 
    {
        return (t1[0] == t2[0]) &&
            (t1[1] == t2[1]) &&
            (t1[2] == t2[2]) &&
            (t1[3] == t2[3]) &&
            (t1[4] == t2[4]) &&
            (t1[5] == t2[5]) &&
            (t1[6] == t2[6]) &&
            (t1[7] == t2[7]) &&
            (t1[8] == t2[8]) &&
            (t1[9] == t2[9]) &&
            (t1[10] == t2[10]) &&
            (t1[11] == t2[11]) &&
            (t1[12] == t2[12]) &&
            (t1[13] == t2[13]) &&
            (t1[14] == t2[14]) &&
            (t1[15] == t2[15]);
    }

    static bool Init() 
    {
        t1 = new float[16];
        t2 = new float[16];
        t1x4 = new float[16];
        t2x4 = new float[16];
        out = new float[16];
        outx4 = new float[16];

        t1[0] = 1.0;
        t1[5] = 1.0;
        t1[10] = 1.0;
        t1[15] = 1.0;

        t2[0] = 2.0;
        t2[5] = 2.0;
        t2[10] = 2.0;
        t2[15] = 2.0;

        t1x4[0] = 1.0;
        t1x4[5] = 1.0;
        t1x4[10] = 1.0;
        t1x4[15] = 1.0;

        t2x4[0] = 2.0;
        t2x4[5] = 2.0;
        t2x4[10] = 2.0;
        t2x4[15] = 2.0;

        Multiply32(1);
        SimdMultiply(1);

        return Equals(t1, t1x4) && Equals(t2, t2x4) && Equals(out, outx4);
    }

    static bool Cleanup() 
    {
        t1[0] = 1.0;
        t1[5] = 1.0;
        t1[10] = 1.0;
        t1[15] = 1.0;

        t2[0] = 2.0;
        t2[5] = 2.0;
        t2[10] = 2.0;
        t2[15] = 2.0;

        t1x4[0] = 1.0;
        t1x4[5] = 1.0;
        t1x4[10] = 1.0;
        t1x4[15] = 1.0;

        t2x4[0] = 2.0;
        t2x4[5] = 2.0;
        t2x4[10] = 2.0;
        t2x4[15] = 2.0;

        Multiply32(1);
        SimdMultiply(1);

        bool ret = Equals(t1, t1x4) && Equals(t2, t2x4) && Equals(out, outx4);

        delete[] t1;
        delete[] t2;
        delete[] t1x4;
        delete[] t2x4;
        delete[] out;
        delete[] outx4;

        return ret;
    };

    static uint64_t Multiply32(uint64_t n)
    {
        for (uint64_t i = 0; i < n; i++) 
        {
            float a00 = t1[0];
            float a01 = t1[1];
            float a02 = t1[2];
            float a03 = t1[3];
            float a10 = t1[4];
            float a11 = t1[5];
            float a12 = t1[6];
            float a13 = t1[7];
            float a20 = t1[8];
            float a21 = t1[9];
            float a22 = t1[10];
            float a23 = t1[11];
            float a30 = t1[12];
            float a31 = t1[13];
            float a32 = t1[14];
            float a33 = t1[15];

            float b0 = t2[0];
            float b1 = t2[1];
            float b2 = t2[2];
            float b3 = t2[3];
            out[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[4];
            b1 = t2[5];
            b2 = t2[6];
            b3 = t2[7];
            out[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[8];
            b1 = t2[9];
            b2 = t2[10];
            b3 = t2[11];
            out[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[12];
            b1 = t2[13];
            b2 = t2[14];
            b3 = t2[15];
            out[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t Multiply64(uint64_t n) 
    {
        for (uint64_t i = 0; i < n; i++)
        {
            double a00 = t1[0];
            double a01 = t1[1];
            double a02 = t1[2];
            double a03 = t1[3];
            double a10 = t1[4];
            double a11 = t1[5];
            double a12 = t1[6];
            double a13 = t1[7];
            double a20 = t1[8];
            double a21 = t1[9];
            double a22 = t1[10];
            double a23 = t1[11];
            double a30 = t1[12];
            double a31 = t1[13];
            double a32 = t1[14];
            double a33 = t1[15];

            double b0 = t2[0];
            double b1 = t2[1];
            double b2 = t2[2];
            double b3 = t2[3];
            out[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[4];
            b1 = t2[5];
            b2 = t2[6];
            b3 = t2[7];
            out[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[8];
            b1 = t2[9];
            b2 = t2[10];
            b3 = t2[11];
            out[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

            b0 = t2[12];
            b1 = t2[13];
            b2 = t2[14];
            b3 = t2[15];
            out[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
            out[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
            out[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
            out[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t SimdMultiply(uint64_t n)
    {
        for (uint64_t i = 0; i < n; i++) 
        {
            __m128 a0 = _mm_loadu_ps(&t1x4[0]);
            __m128 a1 = _mm_loadu_ps(&t1x4[4]);
            __m128 a2 = _mm_loadu_ps(&t1x4[8]);
            __m128 a3 = _mm_loadu_ps(&t1x4[12]);

            __m128 b0 = _mm_loadu_ps(&t2x4[0]);
            _mm_storeu_ps(
                &outx4[0],
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(b0, b0, _MM_SHUFFLE(0, 0, 0, 0)), a0),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(b0, b0, _MM_SHUFFLE(1, 1, 1, 1)), a1),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(b0, b0, _MM_SHUFFLE(2, 2, 2, 2)), a2),
                            _mm_mul_ps(_mm_shuffle_ps(b0, b0, _MM_SHUFFLE(3, 3, 3, 3)), a3)))));

            __m128 b1 = _mm_loadu_ps(&t2x4[4]);
            _mm_storeu_ps(
                &outx4[4],
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(b1, b1, _MM_SHUFFLE(0, 0, 0, 0)), a0),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(b1, b1, _MM_SHUFFLE(1, 1, 1, 1)), a1),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(b1, b1, _MM_SHUFFLE(2, 2, 2, 2)), a2),
                            _mm_mul_ps(_mm_shuffle_ps(b1, b1, _MM_SHUFFLE(3, 3, 3, 3)), a3)))));

            __m128 b2 = _mm_loadu_ps(&t2x4[8]);
            _mm_storeu_ps(
                &outx4[8],
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(b2, b2, _MM_SHUFFLE(0, 0, 0, 0)), a0),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(b2, b2, _MM_SHUFFLE(1, 1, 1, 1)), a1),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(b2, b2, _MM_SHUFFLE(2, 2, 2, 2)), a2),
                            _mm_mul_ps(_mm_shuffle_ps(b2, b2, _MM_SHUFFLE(3, 3, 3, 3)), a3)))));

            __m128 b3 = _mm_loadu_ps(&t2x4[12]);
            _mm_storeu_ps(
                &outx4[12],
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(b3, b3, _MM_SHUFFLE(0, 0, 0, 0)), a0),
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(b3, b3, _MM_SHUFFLE(1, 1, 1, 1)), a1),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(b3, b3, _MM_SHUFFLE(2, 2, 2, 2)), a2),
                            _mm_mul_ps(_mm_shuffle_ps(b3, b3, _MM_SHUFFLE(3, 3, 3, 3)), a3)))));
            preventOptimize++;
        }
        return preventOptimize;
    }
};

uint64_t MatrixMultiplication::preventOptimize = 0;

float* MatrixMultiplication::t1 = NULL;
float* MatrixMultiplication::t2 = NULL;
float* MatrixMultiplication::t1x4 = NULL;
float* MatrixMultiplication::t2x4 = NULL;
float* MatrixMultiplication::out = NULL;
float* MatrixMultiplication::outx4 = NULL;

#endif
