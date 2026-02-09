#include "fixedpoint.h"
/* fixedpoint.c */

int32_t float_to_q31(double x)
{
    return (int32_t)(x * 2147483648.0); 
}