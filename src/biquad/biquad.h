struct biquad_coeffs {
#ifdef FLOATING_POINT
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
#endif
#ifdef FIXED_POINT
    q15 b0;
    q15 b1;
    q15 b2;
    q15 a1;
    q15 a2;
#endif
};