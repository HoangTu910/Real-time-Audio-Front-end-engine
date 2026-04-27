#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtafe/fe_api.h"

int main() {
    printf("Test: RTAFE Initialization Only\n");
    fflush(stdout);
    
    fe_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    
    cfg.sample_rate     = 16000;
    cfg.frame_len       = 256;
    cfg.hop_len         = 128;
    cfg.num_channels    = 2;
    cfg.flags           = FE_FLAG_NOISE_SUPPRESS | FE_FLAG_AGC;
    cfg.dc_rm_alpha     = 0x7FD00000;  /* Set a reasonable alpha */
    cfg.pre_emphasis_alpha = 0x5000;   /* Set a reasonable alpha */
    
    size_t state_sz   = fe_state_bytes(&cfg);
    size_t scratch_sz = fe_scratch_bytes(&cfg);
    
    printf("State size: %zu bytes\n", state_sz);
    printf("Scratch size: %zu bytes\n", scratch_sz);
    fflush(stdout);
    
    void *state_mem   = malloc(state_sz);
    void *scratch_mem = malloc(scratch_sz);
    
    if (!state_mem || !scratch_mem) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Memory allocated successfully\n");
    fflush(stdout);
    
    fe_state_t *state = (fe_state_t *)state_mem;
    printf("Calling fe_init...\n");
    fflush(stdout);
    
    fe_status_t status = fe_init(&cfg, state, scratch_mem, scratch_sz);
    
    printf("fe_init returned: %d\n", status);
    fflush(stdout);
    
    if (status == FE_OK) {
        printf("Initialization successful!\n");
        printf("State frame_len: %u\n", state->frame_len);
        printf("State num_channels: %u\n", state->num_channels);
        printf("State flags: 0x%02X\n", state->flags);
    } else {
        printf("Initialization failed with status: %d\n", status);
    }
    
    free(state_mem);
    free(scratch_mem);
    
    printf("Test completed successfully\n");
    fflush(stdout);
    return 0;
}
