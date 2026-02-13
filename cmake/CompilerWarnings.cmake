# ── Compiler warnings interface target ────────────────────────────────────────
# Usage: target_link_libraries(mytarget PRIVATE rtafe_warnings)

add_library(rtafe_warnings INTERFACE)

target_compile_options(rtafe_warnings INTERFACE
    $<$<C_COMPILER_ID:GNU>:
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion
        -Wdouble-promotion -Wformat=2
        -Wno-unused-parameter
    >
    $<$<C_COMPILER_ID:Clang,AppleClang>:
        -Wall -Wextra -Wpedantic
        -Wshadow -Wconversion -Wsign-conversion
        -Wdouble-promotion -Wformat=2
        -Wno-unused-parameter
    >
    $<$<C_COMPILER_ID:MSVC>:
        /W4
    >
)
