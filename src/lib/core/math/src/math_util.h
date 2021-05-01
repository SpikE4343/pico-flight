#ifndef __math_util_h_INCLUDED__
#define __math_util_h_INCLUDED__

#include "vector.h"

float math_min(float a, float b);
float math_max(float a, float b);

#define math_clamp(m, v, mx) (math_max(m, math_min(v, mx)))

float standardDeviation(Vector3f_t *samples, int axis, int count, float average);

#endif