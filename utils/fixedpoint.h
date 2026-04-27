/* fixedpoint.h — Fixed-point math utilities (saturating ops, Q-format shifts) */
#include <stdint.h>

#define q15 int16_t
#define q31 int32_t

#define FLOAT_TO_Q15(x) ((q15)((x) * 32768.0f))
#define FLOAT_TO_Q31(x) ((q31)((x) * 2147483648.0f))