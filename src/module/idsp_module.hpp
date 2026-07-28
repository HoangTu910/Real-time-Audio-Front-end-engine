#ifndef IDSPMODULE_HPP
#define IDSPMODULE_HPP

#include "buffer_mng.hpp"
#include "i_biquad_design.hpp"
#include "utils.h"

class IDSPModule {
public:
    virtual ~IDSPModule() = default;
    virtual void vProcessBlock(TrplBufferStr *pProcessBuf) = 0;
    virtual void vProcessBlockFix(TrplBufferStr *pProcessBuf) {};
};

#endif /* IDSPMODULE_HPP */