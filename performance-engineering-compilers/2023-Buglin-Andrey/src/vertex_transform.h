#pragma once
#ifndef _VERTEX_TRANSFORM_H
#define _VERTEX_TRANSFORM_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "base.h"

class VertexTransform : public Base::Benchmark 
{
public:
    VertexTransform() 
        : Base::Benchmark(
            new Base::Configuration(
                string("VertexTransform"),
                Init,
                Cleanup,
                SimdVertextTransform,
                VertextTransform32,
                VertextTransform64,
                1000)) {}

    static uint64_t preventOptimize;

    static float* t;
    static float* v;
    static float* out;
    static float* tx4;
    static float* vx4;
    static float* outx4;

    static bool Init()
    {
        t = new float[16];
        v = new float[4];
        out = new float[4];
        tx4 = new float[16];
        vx4 = new float[4];
        outx4 = new float[4];

        t[0] = 1.0;
        t[5] = 1.0;
        t[10] = 1.0;
        t[15] = 1.0;
        v[0] = 1.0;
        v[1] = 2.0;
        v[2] = 3.0;
        v[3] = 1.0;

        tx4[0] = 1.0;
        tx4[5] = 1.0;
        tx4[10] = 1.0;
        tx4[15] = 1.0;
        vx4[0] = 1.0;
        vx4[1] = 2.0;
        vx4[2] = 3.0;
        vx4[3] = 1.0;

        SimdVertextTransform(1);
        VertextTransform32(1);
        return (outx4[0] == out[0]) && (outx4[1] == out[1]) &&
            (outx4[2] == out[2]) && (outx4[3] == out[3]);
    }

    static bool Cleanup()
    {
        t[0] = 1.0;
        t[5] = 1.0;
        t[10] = 1.0;
        t[15] = 1.0;
        v[0] = 1.0;
        v[1] = 2.0;
        v[2] = 3.0;
        v[3] = 1.0;

        tx4[0] = 1.0;
        tx4[5] = 1.0;
        tx4[10] = 1.0;
        tx4[15] = 1.0;
        vx4[0] = 1.0;
        vx4[1] = 2.0;
        vx4[2] = 3.0;
        vx4[3] = 1.0;

        SimdVertextTransform(1);
        VertextTransform32(1);
        bool ret = (outx4[0] == out[0]) && (outx4[1] == out[1]) &&
            (outx4[2] == out[2]) && (outx4[3] == out[3]);

        delete[] t;
        delete[] v;
        delete[] out;
        delete[] tx4;
        delete[] vx4;
        delete[] outx4;

        return ret;
    }

    static uint64_t VertextTransform32(uint64_t n)
    {
        for (uint64_t i = 0; i < n; i++)
        {
            float x = v[0];
            float y = v[1];
            float z = v[2];
            float w = v[3];
            float m0 = t[0];
            float m4 = t[4];
            float m8 = t[8];
            float m12 = t[12];
            out[0] = (m0 * x + m4 * y + m8 * z + m12 * w);
            float m1 = t[1];
            float m5 = t[5];
            float m9 = t[9];
            float m13 = t[13];
            out[1] = (m1 * x + m5 * y + m9 * z + m13 * w);
            float m2 = t[2];
            float m6 = t[6];
            float m10 = t[10];
            float m14 = t[14];
            out[2] = (m2 * x + m6 * y + m10 * z + m14 * w);
            float m3 = t[3];
            float m7 = t[7];
            float m11 = t[11];
            float m15 = t[15];
            out[3] = (m3 * x + m7 * y + m11 * z + m15 * w);
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t VertextTransform64(uint64_t n)
    {
        for (uint64_t i = 0; i < n; i++)
        {
            double x = v[0];
            double y = v[1];
            double z = v[2];
            double w = v[3];
            double m0 = t[0];
            double m4 = t[4];
            double m8 = t[8];
            double m12 = t[12];
            out[0] = (m0 * x + m4 * y + m8 * z + m12 * w);
            double m1 = t[1];
            double m5 = t[5];
            double m9 = t[9];
            double m13 = t[13];
            out[1] = (m1 * x + m5 * y + m9 * z + m13 * w);
            double m2 = t[2];
            double m6 = t[6];
            double m10 = t[10];
            double m14 = t[14];
            out[2] = (m2 * x + m6 * y + m10 * z + m14 * w);
            double m3 = t[3];
            double m7 = t[7];
            double m11 = t[11];
            double m15 = t[15];
            out[3] = (m3 * x + m7 * y + m11 * z + m15 * w);
            preventOptimize++;
        }
        return preventOptimize;
    }

    static uint64_t SimdVertextTransform(uint64_t n)
    {
        for (uint64_t i = 0; i < n; i++)
        {
            __m128 z = _mm_set1_ps(0.0);
            __m128 v = _mm_loadu_ps(&vx4[0]);
            __m128 xxxx = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
            z = _mm_add_ps(z, _mm_mul_ps(xxxx, _mm_loadu_ps(&tx4[0])));
            __m128 yyyy = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
            z = _mm_add_ps(z, _mm_mul_ps(yyyy, _mm_loadu_ps(&tx4[4])));
            __m128 zzzz = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
            z = _mm_add_ps(z, _mm_mul_ps(zzzz, _mm_loadu_ps(&tx4[8])));
            __m128 wwww = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));
            z = _mm_add_ps(z, _mm_mul_ps(wwww, _mm_loadu_ps(&tx4[12])));
            _mm_storeu_ps(&outx4[0], z);
            preventOptimize++;
        }
        return preventOptimize;
    }
};

uint64_t VertexTransform::preventOptimize = 0;

float* VertexTransform::t = NULL;
float* VertexTransform::v = NULL;
float* VertexTransform::out = NULL;
float* VertexTransform::tx4 = NULL;
float* VertexTransform::vx4 = NULL;
float* VertexTransform::outx4 = NULL;

#endif