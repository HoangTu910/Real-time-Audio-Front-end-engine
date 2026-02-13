/**
 * @file rtos_isr_skeleton.c
 * @brief Skeleton showing how to integrate RTAFE into an ISR + RTOS task model.
 *
 * This is a compile-check / reference only — not runnable on a host PC.
 */

#include "rtafe/fe_api.h"
#include <string.h>

/* ── Platform stubs (replace with your RTOS / HAL) ──────────────────────── */

/* Simulated ring buffer between ISR and processing task */
#define RING_FRAMES  4
#define HOP_LEN      128
#define NUM_MICS     2

static int16_t ring_buf[RING_FRAMES][HOP_LEN * NUM_MICS];
static volatile unsigned ring_wr = 0;
static volatile unsigned ring_rd = 0;

/* Called by DMA / I2S interrupt — deposits one hop into the ring buffer */
void i2s_rx_isr(const int16_t *dma_buf, size_t len)
{
    (void)len;
    unsigned wr = ring_wr % RING_FRAMES;
    memcpy(ring_buf[wr], dma_buf, HOP_LEN * NUM_MICS * sizeof(int16_t));
    ring_wr++;
}

/* ── Processing task (runs in RTOS thread context) ──────────────────────── */

int main(void)
{
    /* 1. Configure */
    fe_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.sample_rate  = 16000;
    cfg.frame_len    = 256;
    cfg.hop_len      = HOP_LEN;
    cfg.num_channels = NUM_MICS;
    cfg.flags        = FE_FLAG_NOISE_SUPPRESS | FE_FLAG_AGC;

    /* 2. Allocate (static, no malloc) */
    static uint8_t state_mem[4096];   /* >= fe_state_bytes(&cfg) */
    static uint8_t scratch_mem[8192]; /* >= fe_scratch_bytes(&cfg) */

    fe_state_t *state = (fe_state_t *)state_mem;
    fe_init(&cfg, state, scratch_mem, sizeof(scratch_mem));

    /* 3. Infinite processing loop */
    int16_t pcm_out[HOP_LEN];

    for (;;) {
        /* Wait until ISR has written a new hop */
        while (ring_rd == ring_wr) {
            /* __WFE(); or osThreadYield(); */
        }

        unsigned rd = ring_rd % RING_FRAMES;
        fe_process_hop(state, ring_buf[rd], pcm_out, NULL, 0);
        ring_rd++;

        /* Send pcm_out to DAC / I2S TX / downstream */
    }

    return 0;
}
