/**
 * @file test_sincos_minimal.c
 * @brief Minimal standalone test for fast_sine_cos without Unity framework
 *
 * Compiles and runs without external dependencies:
 *   gcc test_sincos_minimal.c -lm -DFIXED_POINT -o test_sincos_minimal && ./test_sincos_minimal
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* ============================================================================
 * Type Definitions (from utils.h)
 * ============================================================================ */

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

typedef struct {
    float sin, cos;
} sincos;

/* ============================================================================
 * FastSincos Implementation (from utils.h)
 * ============================================================================ */

static inline float fast_sin_poly(float x) {
    /* Polynomial approximation: P5(x) for sin(x) over [-π, π] */
    float x2 = x * x;
    return x * (0.98786f + x2 * (-0.15527f + x2 * 0.005643f));
}

sincos fast_sine_cos(float phase) {
    /**
     * Wrap to [-π, π]
     * This is the optimal range for the polynomial approximation
     */
    if (phase > M_PI) phase -= 2 * M_PI;
    if (phase < -M_PI) phase += 2 * M_PI;

    sincos out;
    out.sin = fast_sin_poly(phase);

    float phase_shift = phase + M_PI_2;
    if (phase_shift > M_PI) phase_shift -= 2 * M_PI;

    out.cos = fast_sin_poly(phase_shift);
    return out;
}

/* ============================================================================
 * Error Tracking
 * ============================================================================ */

typedef struct {
    float max_sin_error;
    float max_cos_error;
    float sum_sin_error_sq;
    float sum_cos_error_sq;
    int num_samples;
} ErrorMetrics;

static void update_metrics(ErrorMetrics *metrics, float phase) {
    sincos result = fast_sine_cos(phase);
    float ref_sin = sinf(phase);
    float ref_cos = cosf(phase);
    
    float sin_error = fabsf(result.sin - ref_sin);
    float cos_error = fabsf(result.cos - ref_cos);
    
    if (sin_error > metrics->max_sin_error) {
        metrics->max_sin_error = sin_error;
    }
    if (cos_error > metrics->max_cos_error) {
        metrics->max_cos_error = cos_error;
    }
    
    metrics->sum_sin_error_sq += sin_error * sin_error;
    metrics->sum_cos_error_sq += cos_error * cos_error;
    metrics->num_samples++;
}

static float calculate_rmse(float sum_error_sq, int num_samples) {
    if (num_samples == 0) return 0.0f;
    return sqrtf(sum_error_sq / num_samples);
}

/* ============================================================================
 * Test Functions
 * ============================================================================ */

void test_special_values(void) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 1: Special Values                                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    struct {
        float phase;
        const char *name;
        float expected_sin;
        float expected_cos;
    } test_cases[] = {
        {0.0f, "Zero", 0.0f, 1.0f},
        {M_PI_2, "π/2", 1.0f, 0.0f},
        {(float)M_PI, "π", 0.0f, -1.0f},
        {-M_PI_2, "-π/2", -1.0f, 0.0f},
    };
    
    int tests_passed = 0;
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        sincos result = fast_sine_cos(test_cases[i].phase);
        float ref_sin = sinf(test_cases[i].phase);
        float ref_cos = cosf(test_cases[i].phase);
        
        float sin_error = fabsf(result.sin - ref_sin);
        float cos_error = fabsf(result.cos - ref_cos);
        
        int pass = (sin_error < 0.01f) && (cos_error < 0.01f);
        tests_passed += pass;
        
        printf("  %-6s: sin=%+.6f (err=%.6f), cos=%+.6f (err=%.6f) [%s]\n",
               test_cases[i].name, result.sin, sin_error, result.cos, cos_error,
               pass ? "PASS" : "FAIL");
    }
    
    printf("\n  Result: %d/%zu tests passed\n", tests_passed, 
           sizeof(test_cases) / sizeof(test_cases[0]));
}

void test_phase_wrapping(void) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 2: Phase Wrapping                                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    struct {
        float phase;
        const char *desc;
    } test_cases[] = {
        {3.0f * (float)M_PI, "3π wraps to π"},
        {-3.0f * (float)M_PI, "-3π wraps to -π"},
        {5.0f * (float)M_PI, "5π wraps to π"},
    };
    
    int tests_passed = 0;
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        sincos result = fast_sine_cos(test_cases[i].phase);
        float ref_sin = sinf(test_cases[i].phase);
        float ref_cos = cosf(test_cases[i].phase);
        
        float sin_error = fabsf(result.sin - ref_sin);
        float cos_error = fabsf(result.cos - ref_cos);
        
        int pass = (sin_error < 0.01f) && (cos_error < 0.01f);
        tests_passed += pass;
        
        printf("  %-20s: sin_err=%.6f, cos_err=%.6f [%s]\n",
               test_cases[i].desc, sin_error, cos_error, pass ? "PASS" : "FAIL");
    }
    
    printf("\n  Result: %d/%zu tests passed\n", tests_passed,
           sizeof(test_cases) / sizeof(test_cases[0]));
}

void test_dense_range(void) {
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 3: Dense Range Sampling [-π, π]                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    ErrorMetrics metrics = {0};
    int num_points = 360;
    
    printf("  Sampling %d points from -π to π...\n", num_points);
    
    for (int i = 0; i < num_points; i++) {
        float phase = -M_PI + (2.0f * M_PI * i) / num_points;
        update_metrics(&metrics, phase);
    }
    
    float rmse_sin = calculate_rmse(metrics.sum_sin_error_sq, metrics.num_samples);
    float rmse_cos = calculate_rmse(metrics.sum_cos_error_sq, metrics.num_samples);
    
    printf("\n  Error Statistics:\n");
    printf("    Sine   - Max: %.8f, RMSE: %.8f\n", metrics.max_sin_error, rmse_sin);
    printf("    Cosine - Max: %.8f, RMSE: %.8f\n", metrics.max_cos_error, rmse_cos);
    printf("\n  Thresholds: Max < 0.01, RMSE < 0.008\n");
    
    int pass = (metrics.max_sin_error < 0.01f) && (metrics.max_cos_error < 0.01f) &&
               (rmse_sin < 0.008f) && (rmse_cos < 0.008f);
    
    printf("  Result: [%s]\n\n", pass ? "PASS" : "FAIL");
}

void test_quadrants(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 4: Quadrant Tests                                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    struct {
        const char *name;
        int start, end, step;
    } quadrants[] = {
        {"Quadrant I   (0° to 90°)", 0, 90, 10},
        {"Quadrant II  (90° to 180°)", 90, 180, 10},
        {"Quadrant III (-180° to -90°)", -180, -90, 10},
        {"Quadrant IV  (-90° to 0°)", -90, 0, 10},
    };
    
    for (size_t q = 0; q < sizeof(quadrants) / sizeof(quadrants[0]); q++) {
        ErrorMetrics metrics = {0};
        int tests = 0;
        
        for (int angle = quadrants[q].start; angle <= quadrants[q].end; 
             angle += quadrants[q].step) {
            float phase = (angle / 180.0f) * (float)M_PI;
            update_metrics(&metrics, phase);
            tests++;
        }
        
        float rmse_sin = calculate_rmse(metrics.sum_sin_error_sq, metrics.num_samples);
        float rmse_cos = calculate_rmse(metrics.sum_cos_error_sq, metrics.num_samples);
        
        int pass = (metrics.max_sin_error < 0.01f) && (metrics.max_cos_error < 0.01f);
        
        printf("  %-25s: Max(%.6f, %.6f) RMSE(%.6f, %.6f) [%s]\n",
               quadrants[q].name, metrics.max_sin_error, metrics.max_cos_error,
               rmse_sin, rmse_cos, pass ? "PASS" : "FAIL");
    }
    
    printf("\n");
}

void test_pythagorean_identity(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 5: Pythagorean Identity (sin² + cos² ≈ 1)            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    float max_identity_error = 0.0f;
    int tests_passed = 0;
    int num_tests = 360;
    
    for (int i = 0; i < num_tests; i++) {
        float angle = (float)i;
        float phase = (angle / 180.0f) * (float)M_PI;
        
        sincos result = fast_sine_cos(phase);
        float identity = result.sin * result.sin + result.cos * result.cos;
        float identity_error = fabsf(identity - 1.0f);
        
        if (identity_error > max_identity_error) {
            max_identity_error = identity_error;
        }
        
        if (identity_error < 0.05f) {
            tests_passed++;
        }
    }
    
    printf("  Tested %d points\n", num_tests);
    printf("  Max identity error: %.8f\n", max_identity_error);
    printf("  Threshold: < 0.05\n");
    printf("  Result: %d/%d tests passed [%s]\n\n", tests_passed, num_tests,
           (tests_passed == num_tests) ? "PASS" : "FAIL");
}

void test_error_table(void) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ TEST 6: Error Distribution Table                           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    float phase_values[] = {
        0.0f,
        0.1f * M_PI,
        0.25f * M_PI,
        0.5f * M_PI,
        0.75f * M_PI,
        (float)M_PI,
        -0.1f * M_PI,
        -0.25f * M_PI,
        -0.5f * M_PI,
        -0.75f * M_PI,
    };
    
    printf("\n  %-12s  %-12s  %-12s  %-12s\n", "Phase (rad)", "Sin Error", "Cos Error", "Pythagorean Err");
    printf("  %-12s  %-12s  %-12s  %-12s\n", "---", "---", "---", "---");
    
    for (size_t i = 0; i < sizeof(phase_values) / sizeof(phase_values[0]); i++) {
        float phase = phase_values[i];
        sincos result = fast_sine_cos(phase);
        
        float ref_sin = sinf(phase);
        float ref_cos = cosf(phase);
        
        float sin_error = fabsf(result.sin - ref_sin);
        float cos_error = fabsf(result.cos - ref_cos);
        float pythagorean_err = fabsf(result.sin * result.sin + result.cos * result.cos - 1.0f);
        
        printf("  %-12.6f  %-12.8f  %-12.8f  %-12.8f\n",
               phase, sin_error, cos_error, pythagorean_err);
    }
    
    printf("\n");
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         FastSincos Polynomial Approximation Test           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_special_values();
    test_phase_wrapping();
    test_dense_range();
    test_quadrants();
    test_pythagorean_identity();
    test_error_table();
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    Test Suite Complete                     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}
