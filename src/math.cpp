#include "glint/math/math.h"

#include <cmath>

float clamp(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}