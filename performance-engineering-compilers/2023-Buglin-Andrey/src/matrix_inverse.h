#pragma once
#ifndef _MATRIX_INVERSE_H
#define _MATRIX_INVERSE_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "base.h"

class MatrixInverse : public Base::Benchmark 
{
public:
    MatrixInverse() 
        : Base::Benchmark(
            new Base::Configuration(
                string("MatrixInverse"),
                Init,
                Cleanup,
                SimdMatrixInverse,
                MatrixInverse32,
                MatrixInverse64,
                1000)) {}

    static uint64_t preventOptimize;

    static float* src;            // Source matrix
    static double* src64;
    static float* srcx4;  // Source matrix
    static float* dst;            // Result matrix
    static double* dst64;
    static float* dstx4;  // Result matrix
    static float* tsrc;            // Transposed version of 'src'
    static double* tsrc64;
    static float* tsrcx4; // Transposed version of 'src'
    static float* tmp;             // Temporary array of multiply results
    static double* tmp64;
    static float* ident;
    static double* ident64;

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

    static void InitMatrix(float* matrix) 
    {
        // These values were chosen somewhat randomly, but they will at least yield a solution.
        matrix[0] = 0;  matrix[1] = 1; matrix[2] = 2; matrix[3] = 3;
        matrix[4] = -1; matrix[5] = -2; matrix[6] = -3; matrix[7] = -4;
        matrix[8] = 0;  matrix[9] = 0; matrix[10] = 2; matrix[11] = 3;
        matrix[12] = -1; matrix[13] = -2; matrix[14] = 0; matrix[15] = -4;
    }

    static void InitMatrix64(double* matrix) 
    {
        // These values were chosen somewhat randomly, but they will at least yield a solution.
        matrix[0] = 0;  matrix[1] = 1; matrix[2] = 2; matrix[3] = 3;
        matrix[4] = -1; matrix[5] = -2; matrix[6] = -3; matrix[7] = -4;
        matrix[8] = 0;  matrix[9] = 0; matrix[10] = 2; matrix[11] = 3;
        matrix[12] = -1; matrix[13] = -2; matrix[14] = 0; matrix[15] = -4;
    }

    static void MulMatrix(float* dst, const float* op1, const float* op2) 
    {
        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c) 
            {
                int ri = 4 * r;
                dst[ri + c] = op1[ri] * op2[c] + op1[ri + 1] * op2[c + 4] + op1[ri + 2] * op2[c + 8] + op1[ri + 3] * op2[c + 12];
            }
        }
    }

    static void MulMatrix64(double* dst, const double* op1, const double* op2) 
    {
        for (int r = 0; r < 4; ++r) 
        {
            for (int c = 0; c < 4; ++c) 
            {
                int ri = 4 * r;
                dst[ri + c] = op1[ri] * op2[c] + op1[ri + 1] * op2[c + 4] + op1[ri + 2] * op2[c + 8] + op1[ri + 3] * op2[c + 12];
            }
        }
    }

    static bool CheckMatrix(float* matrix) 
    {
        // when multiplied with the src matrix it should yield the identity matrix
        MulMatrix(tsrc, src, matrix);
        for (int i = 0; i < 16; ++i) 
        {
            if (fabs(tsrc[i] - ident[i]) > 0.00001)
            {
                return false;
            }
        }
        // PrintMatrix (tsrc);
        return true;
    }

    static bool checkMatrix64(double* matrix) 
    {
        // when multiplied with the src matrix it should yield the identity matrix
        MulMatrix64(tsrc64, src64, matrix);
        for (int i = 0; i < 16; ++i)
        {
            if (fabs(tsrc[i] - ident[i]) > 0.00001) 
            {
                return false;
            }
        }
        // PrintMatrix (tsrc);
        return true;
    }

    static bool Init() 
    {
        src = new float[16];
        srcx4 = src;
        src64 = new double[16];
        dst = new float[16];
        dstx4 = dst;
        dst64 = new double[16];
        tsrc = new float[16];
        tsrc64 = new double[16];
        tsrcx4 = tsrc;
        tmp = new float[12];
        tmp64 = new double[12];
        ident = new float[16];
        ident[0] = 1;
        ident[5] = 1;
        ident[10] = 1;
        ident[15] = 1;
        ident64 = new double[16];
        ident64[0] = 1;
        ident64[5] = 1;
        ident64[10] = 1;
        ident64[15] = 1;

        InitMatrix(src);
        // PrintMatrix(src);
        MatrixInverse32(1);
        // PrintMatrix(dst);
        if (!CheckMatrix(dst))
        {
            return false;
        }

        InitMatrix64(src64);
        // PrintMatrix(src);
        MatrixInverse64(1);
        // PrintMatrix(dst);
        if (!checkMatrix64(dst64))
        {
            return false;
        }

        InitMatrix(src);
        SimdMatrixInverse(1);
        // PrintMatrix(dst);
        if (!CheckMatrix(dst)) 
        {
            return false;
        }

        return true;
    }

    static bool Cleanup()
    {
        bool ret = true;
        InitMatrix(src);
        // PrintMatrix(src);
        MatrixInverse32(1);
        // PrintMatrix(dst);
        if (!CheckMatrix(dst))
        {
            ret = false;
        }

        InitMatrix64(src64);
        // PrintMatrix(src);
        MatrixInverse64(1);
        // PrintMatrix(dst);
        if (!checkMatrix64(dst64)) 
        {
            ret = false;
        }

        InitMatrix(src);
        SimdMatrixInverse(1);
        // PrintMatrix(dst);
        if (!CheckMatrix(dst))
        {
            ret = false;
        }

        delete[] src;
        delete[] src64;
        delete[] dst;
        delete[] dst64;
        delete[] tsrc;
        delete[] tsrc64;
        delete[] tmp;
        delete[] tmp64;
        delete[] ident;
        delete[] ident64;

        return ret;
    }

    static uint64_t MatrixInverse32(uint64_t n)
    {
        for (uint64_t iterations = 0; iterations < n; ++iterations)
        {
            // Transpose the source matrix
            for (int i = 0; i < 4; i++) 
            {
                tsrc[i] = src[i * 4];
                tsrc[i + 4] = src[i * 4 + 1];
                tsrc[i + 8] = src[i * 4 + 2];
                tsrc[i + 12] = src[i * 4 + 3];
            }

            // Calculate pairs for first 8 elements (cofactors)
            tmp[0] = tsrc[10] * tsrc[15];
            tmp[1] = tsrc[11] * tsrc[14];
            tmp[2] = tsrc[9] * tsrc[15];
            tmp[3] = tsrc[11] * tsrc[13];
            tmp[4] = tsrc[9] * tsrc[14];
            tmp[5] = tsrc[10] * tsrc[13];
            tmp[6] = tsrc[8] * tsrc[15];
            tmp[7] = tsrc[11] * tsrc[12];
            tmp[8] = tsrc[8] * tsrc[14];
            tmp[9] = tsrc[10] * tsrc[12];
            tmp[10] = tsrc[8] * tsrc[13];
            tmp[11] = tsrc[9] * tsrc[12];

            // calculate first 8 elements (cofactors)
            dst[0] = tmp[0] * tsrc[5] + tmp[3] * tsrc[6] + tmp[4] * tsrc[7];
            dst[0] -= tmp[1] * tsrc[5] + tmp[2] * tsrc[6] + tmp[5] * tsrc[7];
            dst[1] = tmp[1] * tsrc[4] + tmp[6] * tsrc[6] + tmp[9] * tsrc[7];
            dst[1] -= tmp[0] * tsrc[4] + tmp[7] * tsrc[6] + tmp[8] * tsrc[7];
            dst[2] = tmp[2] * tsrc[4] + tmp[7] * tsrc[5] + tmp[10] * tsrc[7];
            dst[2] -= tmp[3] * tsrc[4] + tmp[6] * tsrc[5] + tmp[11] * tsrc[7];
            dst[3] = tmp[5] * tsrc[4] + tmp[8] * tsrc[5] + tmp[11] * tsrc[6];
            dst[3] -= tmp[4] * tsrc[4] + tmp[9] * tsrc[5] + tmp[10] * tsrc[6];
            dst[4] = tmp[1] * tsrc[1] + tmp[2] * tsrc[2] + tmp[5] * tsrc[3];
            dst[4] -= tmp[0] * tsrc[1] + tmp[3] * tsrc[2] + tmp[4] * tsrc[3];
            dst[5] = tmp[0] * tsrc[0] + tmp[7] * tsrc[2] + tmp[8] * tsrc[3];
            dst[5] -= tmp[1] * tsrc[0] + tmp[6] * tsrc[2] + tmp[9] * tsrc[3];
            dst[6] = tmp[3] * tsrc[0] + tmp[6] * tsrc[1] + tmp[11] * tsrc[3];
            dst[6] -= tmp[2] * tsrc[0] + tmp[7] * tsrc[1] + tmp[10] * tsrc[3];
            dst[7] = tmp[4] * tsrc[0] + tmp[9] * tsrc[1] + tmp[10] * tsrc[2];
            dst[7] -= tmp[5] * tsrc[0] + tmp[8] * tsrc[1] + tmp[11] * tsrc[2];

            // calculate pairs for second 8 elements (cofactors)
            tmp[0] = tsrc[2] * tsrc[7];
            tmp[1] = tsrc[3] * tsrc[6];
            tmp[2] = tsrc[1] * tsrc[7];
            tmp[3] = tsrc[3] * tsrc[5];
            tmp[4] = tsrc[1] * tsrc[6];
            tmp[5] = tsrc[2] * tsrc[5];
            tmp[6] = tsrc[0] * tsrc[7];
            tmp[7] = tsrc[3] * tsrc[4];
            tmp[8] = tsrc[0] * tsrc[6];
            tmp[9] = tsrc[2] * tsrc[4];
            tmp[10] = tsrc[0] * tsrc[5];
            tmp[11] = tsrc[1] * tsrc[4];

            // calculate second 8 elements (cofactors)
            dst[8] = tmp[0] * tsrc[13] + tmp[3] * tsrc[14] + tmp[4] * tsrc[15];
            dst[8] -= tmp[1] * tsrc[13] + tmp[2] * tsrc[14] + tmp[5] * tsrc[15];
            dst[9] = tmp[1] * tsrc[12] + tmp[6] * tsrc[14] + tmp[9] * tsrc[15];
            dst[9] -= tmp[0] * tsrc[12] + tmp[7] * tsrc[14] + tmp[8] * tsrc[15];
            dst[10] = tmp[2] * tsrc[12] + tmp[7] * tsrc[13] + tmp[10] * tsrc[15];
            dst[10] -= tmp[3] * tsrc[12] + tmp[6] * tsrc[13] + tmp[11] * tsrc[15];
            dst[11] = tmp[5] * tsrc[12] + tmp[8] * tsrc[13] + tmp[11] * tsrc[14];
            dst[11] -= tmp[4] * tsrc[12] + tmp[9] * tsrc[13] + tmp[10] * tsrc[14];
            dst[12] = tmp[2] * tsrc[10] + tmp[5] * tsrc[11] + tmp[1] * tsrc[9];
            dst[12] -= tmp[4] * tsrc[11] + tmp[0] * tsrc[9] + tmp[3] * tsrc[10];
            dst[13] = tmp[8] * tsrc[11] + tmp[0] * tsrc[8] + tmp[7] * tsrc[10];
            dst[13] -= tmp[6] * tsrc[10] + tmp[9] * tsrc[11] + tmp[1] * tsrc[8];
            dst[14] = tmp[6] * tsrc[9] + tmp[11] * tsrc[11] + tmp[3] * tsrc[8];
            dst[14] -= tmp[10] * tsrc[11] + tmp[2] * tsrc[8] + tmp[7] * tsrc[9];
            dst[15] = tmp[10] * tsrc[10] + tmp[4] * tsrc[8] + tmp[9] * tsrc[9];
            dst[15] -= tmp[8] * tsrc[9] + tmp[11] * tsrc[10] + tmp[5] * tsrc[8];

            // calculate determinant
            float det = tsrc[0] * dst[0] + tsrc[1] * dst[1] + tsrc[2] * dst[2] + tsrc[3] * dst[3];

            // calculate matrix inverse
            det = 1 / det;
            for (int j = 0; j < 16; j++)
            {
                dst[j] *= det;
            }
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t MatrixInverse64(uint64_t n) 
    {
        for (uint64_t iterations = 0; iterations < n; ++iterations)
        {
            // Transpose the source matrix
            for (int i = 0; i < 4; i++) 
            {
                tsrc64[i] = src64[i * 4];
                tsrc64[i + 4] = src64[i * 4 + 1];
                tsrc64[i + 8] = src64[i * 4 + 2];
                tsrc64[i + 12] = src64[i * 4 + 3];
            }

            // Calculate pairs for first 8 elements (cofactors)
            tmp64[0] = tsrc64[10] * tsrc64[15];
            tmp64[1] = tsrc64[11] * tsrc64[14];
            tmp64[2] = tsrc64[9] * tsrc64[15];
            tmp64[3] = tsrc64[11] * tsrc64[13];
            tmp64[4] = tsrc64[9] * tsrc64[14];
            tmp64[5] = tsrc64[10] * tsrc64[13];
            tmp64[6] = tsrc64[8] * tsrc64[15];
            tmp64[7] = tsrc64[11] * tsrc64[12];
            tmp64[8] = tsrc64[8] * tsrc64[14];
            tmp64[9] = tsrc64[10] * tsrc64[12];
            tmp64[10] = tsrc64[8] * tsrc64[13];
            tmp64[11] = tsrc64[9] * tsrc64[12];

            // calculate first 8 elements (cofactors)
            dst64[0] = tmp64[0] * tsrc64[5] + tmp64[3] * tsrc64[6] + tmp64[4] * tsrc64[7];
            dst64[0] -= tmp64[1] * tsrc64[5] + tmp64[2] * tsrc64[6] + tmp64[5] * tsrc64[7];
            dst64[1] = tmp64[1] * tsrc64[4] + tmp64[6] * tsrc64[6] + tmp64[9] * tsrc64[7];
            dst64[1] -= tmp64[0] * tsrc64[4] + tmp64[7] * tsrc64[6] + tmp64[8] * tsrc64[7];
            dst64[2] = tmp64[2] * tsrc64[4] + tmp64[7] * tsrc64[5] + tmp64[10] * tsrc64[7];
            dst64[2] -= tmp64[3] * tsrc64[4] + tmp64[6] * tsrc64[5] + tmp64[11] * tsrc64[7];
            dst64[3] = tmp64[5] * tsrc64[4] + tmp64[8] * tsrc64[5] + tmp64[11] * tsrc64[6];
            dst64[3] -= tmp64[4] * tsrc64[4] + tmp64[9] * tsrc64[5] + tmp64[10] * tsrc64[6];
            dst64[4] = tmp64[1] * tsrc64[1] + tmp64[2] * tsrc64[2] + tmp64[5] * tsrc64[3];
            dst64[4] -= tmp64[0] * tsrc64[1] + tmp64[3] * tsrc64[2] + tmp64[4] * tsrc64[3];
            dst64[5] = tmp64[0] * tsrc64[0] + tmp64[7] * tsrc64[2] + tmp64[8] * tsrc64[3];
            dst64[5] -= tmp64[1] * tsrc64[0] + tmp64[6] * tsrc64[2] + tmp64[9] * tsrc64[3];
            dst64[6] = tmp64[3] * tsrc64[0] + tmp64[6] * tsrc64[1] + tmp64[11] * tsrc64[3];
            dst64[6] -= tmp64[2] * tsrc64[0] + tmp64[7] * tsrc64[1] + tmp64[10] * tsrc64[3];
            dst64[7] = tmp64[4] * tsrc64[0] + tmp64[9] * tsrc64[1] + tmp64[10] * tsrc64[2];
            dst64[7] -= tmp64[5] * tsrc64[0] + tmp64[8] * tsrc64[1] + tmp64[11] * tsrc64[2];

            // calculate pairs for second 8 elements (cofactors)
            tmp64[0] = tsrc64[2] * tsrc64[7];
            tmp64[1] = tsrc64[3] * tsrc64[6];
            tmp64[2] = tsrc64[1] * tsrc64[7];
            tmp64[3] = tsrc64[3] * tsrc64[5];
            tmp64[4] = tsrc64[1] * tsrc64[6];
            tmp64[5] = tsrc64[2] * tsrc64[5];
            tmp64[6] = tsrc64[0] * tsrc64[7];
            tmp64[7] = tsrc64[3] * tsrc64[4];
            tmp64[8] = tsrc64[0] * tsrc64[6];
            tmp64[9] = tsrc64[2] * tsrc64[4];
            tmp64[10] = tsrc64[0] * tsrc64[5];
            tmp64[11] = tsrc64[1] * tsrc64[4];

            // calculate second 8 elements (cofactors)
            dst64[8] = tmp64[0] * tsrc64[13] + tmp64[3] * tsrc64[14] + tmp64[4] * tsrc64[15];
            dst64[8] -= tmp64[1] * tsrc64[13] + tmp64[2] * tsrc64[14] + tmp64[5] * tsrc64[15];
            dst64[9] = tmp64[1] * tsrc64[12] + tmp64[6] * tsrc64[14] + tmp64[9] * tsrc64[15];
            dst64[9] -= tmp64[0] * tsrc64[12] + tmp64[7] * tsrc64[14] + tmp64[8] * tsrc64[15];
            dst64[10] = tmp64[2] * tsrc64[12] + tmp64[7] * tsrc64[13] + tmp64[10] * tsrc64[15];
            dst64[10] -= tmp64[3] * tsrc64[12] + tmp64[6] * tsrc64[13] + tmp64[11] * tsrc64[15];
            dst64[11] = tmp64[5] * tsrc64[12] + tmp64[8] * tsrc64[13] + tmp64[11] * tsrc64[14];
            dst64[11] -= tmp64[4] * tsrc64[12] + tmp64[9] * tsrc64[13] + tmp64[10] * tsrc64[14];
            dst64[12] = tmp64[2] * tsrc64[10] + tmp64[5] * tsrc64[11] + tmp64[1] * tsrc64[9];
            dst64[12] -= tmp64[4] * tsrc64[11] + tmp64[0] * tsrc64[9] + tmp64[3] * tsrc64[10];
            dst64[13] = tmp64[8] * tsrc64[11] + tmp64[0] * tsrc64[8] + tmp64[7] * tsrc64[10];
            dst64[13] -= tmp64[6] * tsrc64[10] + tmp64[9] * tsrc64[11] + tmp64[1] * tsrc64[8];
            dst64[14] = tmp64[6] * tsrc64[9] + tmp64[11] * tsrc64[11] + tmp64[3] * tsrc64[8];
            dst64[14] -= tmp64[10] * tsrc64[11] + tmp64[2] * tsrc64[8] + tmp64[7] * tsrc64[9];
            dst64[15] = tmp64[10] * tsrc64[10] + tmp64[4] * tsrc64[8] + tmp64[9] * tsrc64[9];
            dst64[15] -= tmp64[8] * tsrc64[9] + tmp64[11] * tsrc64[10] + tmp64[5] * tsrc64[8];

            // calculate determinant
            double det = tsrc64[0] * dst64[0] + tsrc64[1] * dst64[1] + tsrc64[2] * dst64[2] + tsrc64[3] * dst64[3];

            // calculate matrix inverse
            det = 1 / det;
            for (int j = 0; j < 16; j++) 
            {
                dst64[j] *= det;
            }
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t SimdMatrixInverse(uint64_t n)
    {
        for (uint64_t iterations = 0; iterations < n; ++iterations)
        {
            __m128 src0, src1, src2, src3;
            __m128 row0, row1, row2, row3;
            __m128 tmp1;
            __m128 minor0, minor1, minor2, minor3;
            __m128 det;

            // Load the 4 rows
            src0 = _mm_loadu_ps(&srcx4[0]);
            src1 = _mm_loadu_ps(&srcx4[4]);
            src2 = _mm_loadu_ps(&srcx4[8]);
            src3 = _mm_loadu_ps(&srcx4[12]);

            // Transpose the source matrix.  Sort of.  Not a true transpose operation
            tmp1 = _mm_shuffle_ps(src0, src1, _MM_SHUFFLE(1, 0, 1, 0));
            row1 = _mm_shuffle_ps(src2, src3, _MM_SHUFFLE(1, 0, 1, 0));
            row0 = _mm_shuffle_ps(tmp1, row1, _MM_SHUFFLE(2, 0, 2, 0));
            row1 = _mm_shuffle_ps(row1, tmp1, _MM_SHUFFLE(3, 1, 3, 1));

            tmp1 = _mm_shuffle_ps(src0, src1, _MM_SHUFFLE(3, 2, 3, 2));
            row3 = _mm_shuffle_ps(src2, src3, _MM_SHUFFLE(3, 2, 3, 2));
            row2 = _mm_shuffle_ps(tmp1, row3, _MM_SHUFFLE(2, 0, 2, 0));
            row3 = _mm_shuffle_ps(row3, tmp1, _MM_SHUFFLE(3, 1, 3, 1));

            // ----
            tmp1 = _mm_mul_ps(row2, row3);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            minor0 = _mm_mul_ps(row1, tmp1);
            minor1 = _mm_mul_ps(row0, tmp1);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
            minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
            minor1 = _mm_shuffle_ps(minor1, minor1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110

            // ----
            tmp1 = _mm_mul_ps(row1, row2);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
            minor3 = _mm_mul_ps(row0, tmp1);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
            minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
            minor3 = _mm_shuffle_ps(minor3, minor3, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110

            // ----
            tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, _MM_SHUFFLE(1, 0, 3, 2)), row3); // 0x4E = 01001110
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            row2 = _mm_shuffle_ps(row2, row2, _MM_SHUFFLE(1, 0, 3, 2));  // 0x4E = 01001110
            minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
            minor2 = _mm_mul_ps(row0, tmp1);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
            minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
            minor2 = _mm_shuffle_ps(minor2, minor2, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110

            // ----
            tmp1 = _mm_mul_ps(row0, row1);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
            minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
            minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

            // ----
            tmp1 = _mm_mul_ps(row0, row3);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
            minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
            minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

            // ----
            tmp1 = _mm_mul_ps(row0, row2);
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(2, 3, 0, 1)); // 0xB1 = 10110001
            minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
            minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
            tmp1 = _mm_shuffle_ps(tmp1, tmp1, _MM_SHUFFLE(1, 0, 3, 2)); // 0x4E = 01001110
            minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
            minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

            // Compute determinant
            det = _mm_mul_ps(row0, minor0);
            det = _mm_add_ps(_mm_shuffle_ps(det, det, _MM_SHUFFLE(1, 0, 3, 2)), det); // 0x4E = 01001110
            det = _mm_add_ps(_mm_shuffle_ps(det, det, _MM_SHUFFLE(2, 3, 0, 1)), det); // 0xB1 = 10110001
            tmp1 = _mm_rcp_ps(det);
            det = _mm_sub_ps(_mm_add_ps(tmp1, tmp1), _mm_mul_ps(det, _mm_mul_ps(tmp1, tmp1)));
            det = _mm_shuffle_ps(det, det, _MM_SHUFFLE(0, 0, 0, 0));

            // Compute final values by multiplying with 1/det
            minor0 = _mm_mul_ps(det, minor0);
            minor1 = _mm_mul_ps(det, minor1);
            minor2 = _mm_mul_ps(det, minor2);
            minor3 = _mm_mul_ps(det, minor3);

            _mm_storeu_ps(&dstx4[0], minor0);
            _mm_storeu_ps(&dstx4[4], minor1);
            _mm_storeu_ps(&dstx4[8], minor2);
            _mm_storeu_ps(&dstx4[12], minor3);
            preventOptimize++;
        }
        return preventOptimize;
    }
};

uint64_t MatrixInverse::preventOptimize = 0;

float* MatrixInverse::src = NULL;
double* MatrixInverse::src64 = NULL;
float* MatrixInverse::srcx4 = NULL;
float* MatrixInverse::dst = NULL;
double* MatrixInverse::dst64 = NULL;
float* MatrixInverse::dstx4 = NULL;
float* MatrixInverse::tsrc = NULL;
double* MatrixInverse::tsrc64 = NULL;
float* MatrixInverse::tsrcx4 = NULL;
float* MatrixInverse::tmp = NULL;
double* MatrixInverse::tmp64 = NULL;
float* MatrixInverse::ident = NULL;
double* MatrixInverse::ident64 = NULL;

#endif
