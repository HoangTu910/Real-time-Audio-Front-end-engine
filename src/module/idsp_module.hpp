#ifndef IDSPMODULE_HPP
#define IDSPMODULE_HPP

#include "buffer_pool.hpp"
#include "i_biquad_design.hpp"
#include "utils.hpp"

class IDSPModule {
public:
    virtual ~IDSPModule() = default;
    virtual void ProcessBlock(DSPBlock *dsp_block) = 0;
    virtual void ProcessBlockFixed(DSPBlock *dsp_block) = 0;
};

#endif /* IDSPMODULE_HPP */