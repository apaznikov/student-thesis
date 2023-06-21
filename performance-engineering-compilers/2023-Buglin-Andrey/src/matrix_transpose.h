#pragma once
#ifndef _MATRIX_TRANSPOSE_H
#define _MATRIX_TRANSPOSE_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "base.h"

class MatrixTranspose : public Base::Benchmark 
{
public:
    MatrixTranspose() 
        : Base::Benchmark(
            new Base::Configuration(
                string("MatrixTranspose"),
                Init,
                Cleanup,
                SimdTranspose,
                Transpose32,
                Transpose64,
                1000)) {}

    static uint64_t preventOptimize;

    static float* src;
    static float* srcx4;
    static float* dst;
    static float* dstx4;
    static float* tsrc;
    static float* tsrcx4;

    static void PrintMatrix(const float* matrix) 
    {
        for (int r = 0; r < 4; ++r) 
        {
            int ri = r * 4;
            for (int c = 0; c < 4; ++c) 
            {
                float value = matrix[ri + c];
                printf("%f ", value);
            }
            printf("\n");
        }
        printf("\n");
    }

    static void InitMatrix(float* matrix, float* matrixTransposed) 
    {
        for (int r = 0; r < 4; ++r) 
        {
            int r4 = 4 * r;
            for (int c = 0; c < 4; ++c) 
            {
                matrix[r4 + c] = r4 + c;
                matrixTransposed[r + c * 4] = r4 + c;
            }
        }
    }

    static bool CompareEqualMatrix(const float* m1, const float* m2) 
    {
        for (int i = 0; i < 16; ++i)
        {
            if (m1[i] != m2[i])
            {
                return false;
            }
        }
        return true;
    }

    static bool Init() 
    {
        src = new float[16];
        srcx4 = src;
        dst = new float[16];
        dstx4 = dst;
        tsrc = new float[16];
        tsrcx4 = tsrc;

        InitMatrix(src, tsrc);
        Transpose32(1);

        if (!CompareEqualMatrix(tsrc, dst)) 
        {
            return false;
        }

        SimdTranspose(1);

        if (!CompareEqualMatrix(tsrc, dst)) 
        {
            return false;
        }

        return true;
    }

    static bool Cleanup()
    {
        bool ret = true;
        InitMatrix(src, tsrc);
        Transpose32(1);

        if (!CompareEqualMatrix(tsrc, dst))
        {
            ret = false;
        }

        SimdTranspose(1);

        if (!CompareEqualMatrix(tsrc, dst)) 
        {
            ret = false;
        }

        delete[] src;
        delete[] dst;
        delete[] tsrc;

        return ret;
    }

    static uint64_t Transpose32(uint64_t n) 
    {
        for (uint64_t i = 0; i < n; ++i)
        {
            dst[0] = src[0];
            dst[1] = src[4];
            dst[2] = src[8];
            dst[3] = src[12];
            dst[4] = src[1];
            dst[5] = src[5];
            dst[6] = src[9];
            dst[7] = src[13];
            dst[8] = src[2];
            dst[9] = src[6];
            dst[10] = src[10];
            dst[11] = src[14];
            dst[12] = src[3];
            dst[13] = src[7];
            dst[14] = src[11];
            dst[15] = src[15];
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t Transpose64(uint64_t n) 
    {
        for (uint64_t i = 0; i < n; ++i) 
        {
            dst[0] = src[0];
            dst[1] = src[4];
            dst[2] = src[8];
            dst[3] = src[12];
            dst[4] = src[1];
            dst[5] = src[5];
            dst[6] = src[9];
            dst[7] = src[13];
            dst[8] = src[2];
            dst[9] = src[6];
            dst[10] = src[10];
            dst[11] = src[14];
            dst[12] = src[3];
            dst[13] = src[7];
            dst[14] = src[11];
            dst[15] = src[15];
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t SimdTranspose(uint64_t n) 
    {
        for (uint64_t i = 0; i < n; ++i)
        {
            __m128 src0 = _mm_loadu_ps(&srcx4[0]);
            __m128 src1 = _mm_loadu_ps(&srcx4[4]);
            __m128 src2 = _mm_loadu_ps(&srcx4[8]);
            __m128 src3 = _mm_loadu_ps(&srcx4[12]);

            __m128 tmp01 = _mm_shuffle_ps(src0, src1, _MM_SHUFFLE(1, 0, 1, 0));
            __m128 tmp23 = _mm_shuffle_ps(src2, src3, _MM_SHUFFLE(1, 0, 1, 0));
            __m128 dst0 = _mm_shuffle_ps(tmp01, tmp23, _MM_SHUFFLE(2, 0, 2, 0));
            __m128 dst1 = _mm_shuffle_ps(tmp01, tmp23, _MM_SHUFFLE(3, 1, 3, 1));

            tmp01 = _mm_shuffle_ps(src0, src1, _MM_SHUFFLE(3, 2, 3, 2));
            tmp23 = _mm_shuffle_ps(src2, src3, _MM_SHUFFLE(3, 2, 3, 2));
            __m128 dst2 = _mm_shuffle_ps(tmp01, tmp23, _MM_SHUFFLE(2, 0, 2, 0));
            __m128 dst3 = _mm_shuffle_ps(tmp01, tmp23, _MM_SHUFFLE(3, 1, 3, 1));

            _mm_storeu_ps(&dstx4[0], dst0);
            _mm_storeu_ps(&dstx4[4], dst1);
            _mm_storeu_ps(&dstx4[8], dst2);
            _mm_storeu_ps(&dstx4[12], dst3);

            preventOptimize++;
        }
        return preventOptimize;
    }
};

uint64_t MatrixTranspose::preventOptimize = 0;

float* MatrixTranspose::src = NULL;
float* MatrixTranspose::srcx4 = NULL;
float* MatrixTranspose::dst = NULL;
float* MatrixTranspose::dstx4 = NULL;
float* MatrixTranspose::tsrc = NULL;
float* MatrixTranspose::tsrcx4 = NULL;

#endif