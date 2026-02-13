# ── RTAFE unit tests ─────────────────────────────────────────────────────────
# Uses Unity test framework (vendored in tests/unity/ or fetched via CMake).
# Each test_*.c is registered as a separate CTest.

# ── Fetch Unity if not vendored ─────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

# ── Test sources ─────────────────────────────────────────────────────────────
set(RTAFE_TEST_SOURCES
    test_dc_removal.c
    test_preemphasis.c
    test_fft_ifft.c
    test_noise_suppress.c
    test_agc.c
    test_pipeline_e2e.c
    test_scratch_layout.c
)

foreach(test_src IN LISTS RTAFE_TEST_SOURCES)
    # Derive target name: test_dc_removal.c → test_dc_removal
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name} ${test_src})

    target_link_libraries(${test_name}
        PRIVATE
            rtafe_core
            unity
    )

    target_include_directories(${test_name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
