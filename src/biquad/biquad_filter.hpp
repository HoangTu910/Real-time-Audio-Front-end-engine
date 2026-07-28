#include "i_biquad_design.hpp"

class BiquadLPF : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};

class BiquadHPF : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};  

class BiquadPeak : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};

class BiquadBPF : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};

class BiquadNotch : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};

class BiquadAllpass : public IBiquad {
public:
    void Design(float frequency, float q_factor, float sample_rate) override;
};


