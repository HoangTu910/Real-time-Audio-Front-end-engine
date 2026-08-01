#ifndef IDSPMODULE_HPP
#define IDSPMODULE_HPP

#include "buffer_pool.hpp"
#include "ibiquad.hpp"
#include "utils.hpp"
#include "dsp_block.hpp"

class IDSPModule {
public:
    virtual ~IDSPModule() = default;
    virtual void ProcessBlock(DSPBlock *dsp_block) = 0;
    virtual void ProcessBlockFixed(DSPBlock *dsp_block) = 0;
};

#endif /* IDSPMODULE_HPP */