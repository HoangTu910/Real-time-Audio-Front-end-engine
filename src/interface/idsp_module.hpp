#ifndef IDSPMODULE_HPP
#define IDSPMODULE_HPP

#include "utils.hpp"
#include "dsp_block.hpp"
#include "errors_code.hpp"

using DSPModuleParams = float;

class IDSPModule {
public:
    virtual ~IDSPModule() = default;
    virtual HtspErrRet SetParams(const DSPModuleParams *params,
                                 u16 param_count) = 0;
    virtual void ProcessBlock(DSPBlock *dsp_block) = 0;
    virtual void ProcessBlockFixed(DSPBlock *dsp_block) = 0;
    virtual void SetDSPModule(IDSPModule *module);

private:
    IDSPModule *dsp_module_;
};

#endif /* IDSPMODULE_HPP */