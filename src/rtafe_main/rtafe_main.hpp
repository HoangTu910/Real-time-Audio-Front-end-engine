#ifndef RTAFE_MAIN_SP_HPP
#define RTAFE_MAIN_SP_HPP

#include "buffer_pool.hpp"
#include "ibiquad.hpp"
#include "idsp_module.hpp"
#include "dsp_pipeline.hpp"
#include "utils.hpp"
#include "module/pre_emphasis.hpp"
#include "module/dc_removal.hpp"
#include "module/noise_suppress.hpp"

#define BLOCK_SIZE 512
#define NUM_BLOCKS 5

typedef sample_t tSample;

class RTAFE_DSPMain {
public:
    RTAFE_DSPMain();
    ~RTAFE_DSPMain();

    void GetInputBuffer();
    /* main process function with full dsp pipeline*/
    void ProcessDSPBlock(DSPBlock *dsp_block);
private:
    DSPPipeline *dsp_pipeline  = nullptr;
};

#endif /* RTAFE_MAIN_SP_HPP */