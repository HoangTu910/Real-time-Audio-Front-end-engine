#!/bin/bash

set -e

ARCH="x86"
TYPE="FIXED_POINT"

for arg in "$@"; do
    case "$arg" in
        --arch=*)
            ARCH="${arg#*=}"
            ;;
        --type=*)
            TYPE="${arg#*=}"
            ;;
        -h|--help)
            echo "Usage: ./build.sh --arch=x86|arm --type=FIXED_POINT|FLOATING_POINT"
            echo ""
            echo "Builds test_rtafe (WAV processing + triple buffer test)"
            echo ""
            echo "Options:"
            echo "  --arch=x86|arm           Target architecture (default: x86)"
            echo "  --type=FIXED_POINT|FLOATING_POINT  Sample type (default: FIXED_POINT)"
            echo ""
            echo "Run modes:"
            echo "  ./bin/test_rtafe                    Triple buffer test"
            echo "  ./bin/test_rtafe input.wav out.wav  WAV processing"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            exit 1
            ;;
    esac
done

SRC_DIR="src"
UTILS_DIR="utils"

INC_DIRS="-I${SRC_DIR} -I${UTILS_DIR} -I${SRC_DIR}/biquad -I${SRC_DIR}/module -I${SRC_DIR}/buffer"

CPP_SRCS="tests/test_rtafe.cpp \
    ${SRC_DIR}/RTAFE_main_sp.cpp \
    ${SRC_DIR}/RTAFE_main_ap.cpp \
    ${SRC_DIR}/buffer/BufferMng.cpp \
    ${SRC_DIR}/buffer/WavFileMgr.cpp \
    ${SRC_DIR}/biquad/BiquadFilter.cpp \
    ${SRC_DIR}/biquad/IBiquadDesign.cpp \
    ${SRC_DIR}/module/noise_suppress.cpp \
    ${SRC_DIR}/module/dc_removal.cpp \
    ${SRC_DIR}/module/pre-emphasis.cpp \
    ${SRC_DIR}/module/IDSPModule.cpp"

C_SRCS="${SRC_DIR}/module/fft.c"

case "$TYPE" in
    FIXED_POINT)
        TYPE_FLAGS="-DFIXED_POINT"
        ;;
    FLOATING_POINT)
        TYPE_FLAGS=""
        ;;
    *)
        echo "Unsupported type: $TYPE"
        exit 1
        ;;
esac

mkdir -p bin

if [ "$ARCH" = "arm" ]; then
    CXX="aarch64-linux-gnu-g++"
    CC="aarch64-linux-gnu-gcc"
    TARGET_FLAGS="-march=armv8-a+simd -ffast-math -DARM_TARGET"
    OUT="bin/arm_test_rtafe"
    LDFLAGS="-static -lm -lstdc++"
elif [ "$ARCH" = "x86" ]; then
    CXX="g++"
    CC="gcc"
    TARGET_FLAGS=""
    OUT="bin/test_rtafe"
    LDFLAGS="-lm -lstdc++"
else
    echo "Unsupported arch: $ARCH"
    exit 1
fi

echo "Building $OUT (arch=$ARCH, type=$TYPE)"

# Compile C sources
C_OBJS=""
for src in $C_SRCS; do
    obj="${src%.c}.o"
    $CC -Wall -O2 $TYPE_FLAGS $TARGET_FLAGS $INC_DIRS -c "$src" -o "$obj"
    C_OBJS="$C_OBJS $obj"
done

# Compile C++ sources
CXX_OBJS=""
for src in $CPP_SRCS; do
    obj="${src%.cpp}.o"
    $CXX -Wall -O2 -std=c++14 $TYPE_FLAGS $TARGET_FLAGS $INC_DIRS -c "$src" -o "$obj"
    CXX_OBJS="$CXX_OBJS $obj"
done

# Link
$CXX $CXX_OBJS $C_OBJS -o "$OUT" $LDFLAGS

echo "Done: $OUT"