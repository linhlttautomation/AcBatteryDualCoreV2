/*
 * sin_cos_taylor.h
 *
 *  Created on: Nov 6, 2025
 *      Author: LUU LINH K65
 */

#ifndef LIBRARY_CLAMATH_INCLUDE_SIN_COS_TAYLOR_H_
#define LIBRARY_CLAMATH_INCLUDE_SIN_COS_TAYLOR_H_

#include "FLC_Setting.h"

#define TABLE_SIZE 1024
float sin_table[TABLE_SIZE];

static float sinPU_taylor(float pu)
{
    while (pu >= 1.0f) pu -= 1.0f;
    while (pu < 0.0f)  pu += 1.0f;

    float x = pu * TWO_PI;

    if (x > 3.141592653589793f)
        x -= TWO_PI;

    float x2 = x * x;
    float sinx = x;
    float term = x;

    term *= -x2 / (2.0f * 3.0f);
    sinx += term;
    term *= -x2 / (4.0f * 5.0f);
    sinx += term;
    term *= -x2 / (6.0f * 7.0f);
    sinx += term;
    term *= -x2 / (8.0f * 9.0f);
    sinx += term;

    return sinx;
}

static inline float cosPU_taylor(float pu)
{
    return sinPU_taylor(pu + 0.25f);
}

#endif /* LIBRARY_CLAMATH_INCLUDE_SIN_COS_TAYLOR_H_ */
