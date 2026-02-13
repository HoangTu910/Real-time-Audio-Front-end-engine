/* window.h — Windowing stage (apply precomputed analysis window) */
#pragma once
#include "rtafe/fe_types.h"
#include "math/tables.h"
#include <stdint.h>

void window_apply(const q15_t *window, q15_t *frame, size_t frame_len);