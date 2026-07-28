#include "i_biquad_design.hpp"

class BiquadLPF : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};

class BiquadHPF : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};  

class BiquadPeak : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};

class BiquadBPF : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};

class BiquadNotch : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};

class BiquadAllpass : public IBiquadDesign {
public:
    void Design(float frequency, float QFactor, float sampleRate) override;
};


