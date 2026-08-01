#ifndef IDSPMODULE_HPP
#define IDSPMODULE_HPP

#include "utils.hpp"
#include "dsp_block.hpp"

class IDSPModule {
public:
    virtual ~IDSPModule() = default;
    virtual void ProcessBlock(DSPBlock *dsp_block) = 0;
    virtual void ProcessBlockFixed(DSPBlock *dsp_block) = 0;
    virtual void SetDSPModule(IDSPModule *module);

private:
    IDSPModule *dsp_module_;
};

#endif /* IDSPMODULE_HPP */