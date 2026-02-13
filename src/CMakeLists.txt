# ── RTAFE core static library ────────────────────────────────────────────────

set(RTAFE_SOURCES
    fe_api.c
    fe_init.c

    # Pipeline stages
    pipeline/dc_removal.c
    pipeline/preemphasis.c
    pipeline/window.c
    pipeline/fft.c
    pipeline/ifft.c
    pipeline/noise_suppress.c
    pipeline/gain_smooth.c
    pipeline/agc.c
    pipeline/beamformer.c
    pipeline/aec_stub.c

    # Feature extraction
    features/log_mel.c
    features/energy.c

    # Fixed-point math utilities
    math/fixedpoint.c
    math/tables.c

    # Platform abstraction (generic C fallback only — others added per toolchain)
    platform/platform_generic.c
)

add_library(rtafe_core STATIC ${RTAFE_SOURCES})

target_include_directories(rtafe_core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/rtafe>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pipeline>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/math
        ${CMAKE_CURRENT_SOURCE_DIR}/features
        ${CMAKE_CURRENT_SOURCE_DIR}/platform
)

target_link_libraries(rtafe_core PRIVATE rtafe_warnings)

if(RTAFE_ENABLE_SIMD)
    target_compile_definitions(rtafe_core PRIVATE RTAFE_SIMD_ENABLED=1)
endif()
