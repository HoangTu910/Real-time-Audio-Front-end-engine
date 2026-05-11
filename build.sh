#!/bin/bash

set -e

ARCH="x86"
TYPE="FIXED_POINT"
OPT="default"
BENCHMARK=false

for arg in "$@"; do
    case "$arg" in
        --arch=*)
            ARCH="${arg#*=}"
            ;;
        --type=*)
            TYPE="${arg#*=}"
            ;;
        --opt=*)
            OPT="${arg#*=}"
            ;;
        --benchmark)
            BENCHMARK=true
            ;;
        -h|--help)
            echo "Usage: ./build --arch=x86|arm --type=FIXED_POINT|FLOATING_POINT --opt=default|optimize"
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            exit 1
            ;;
    esac
done

INC_DIRS="-Iinclude -Isrc -Iutils -Isrc/biquad"
SRCS="tests/test_benchmark.c src/fe_api.c src/module/dc_removal.c src/biquad/biquad.c"

case "$OPT" in
    default)
        OPT_FLAGS="-O2"
        ;;
    optimize)
        OPT_FLAGS="-O2 -ffast-math -DOPTIMIZATION_METHOD"
        ;;
    *)
        echo "Unsupported optimization level: $OPT"
        exit 1
        ;;
esac

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

if [ "$ARCH" = "arm" ]; then
    CC="aarch64-linux-gnu-gcc"
    TARGET_FLAGS="-march=armv8-a+simd -DARM_TARGET"
    OUT="bin/arm_test_benchmark"
    LDFLAGS="-static"
elif [ "$ARCH" = "x86" ]; then
    CC="gcc"
    TARGET_FLAGS=""
    OUT="bin/test_benchmark"
    LDFLAGS=""
else
    echo "Unsupported arch: $ARCH"
    exit 1
fi

if [ "$BENCHMARK" = "true" ]; then
    EXTRA_FLAGS="$EXTRA_FLAGS -DBENCHMARK_MODE"
fi

mkdir -p bin

echo "Building $OUT (arch=$ARCH, type=$TYPE, opt=$OPT, benchmark=$BENCHMARK)"
$CC -Wall $OPT_FLAGS $TARGET_FLAGS $TYPE_FLAGS $EXTRA_FLAGS $INC_DIRS $SRCS -o "$OUT" $LDFLAGS -lm