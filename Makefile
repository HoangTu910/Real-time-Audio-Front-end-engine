SRC_DIR = src
UTILS_DIR = utils
BIQUAD_DIR = src/biquad
ARM_CORTEX_M_DIR = arm-cortexM

TARGET = biquad_test
TARGET_ARM = biquad_test_arm

# Host compiler
CC = gcc

# ARMv8-A compiler (AArch64)
CC_ARM = aarch64-linux-gnu-gcc

INC_DIRS = -Iinclude -I$(SRC_DIR) -I$(UTILS_DIR) -I$(BIQUAD_DIR)

LDLIBS = -lm
CFLAGS = -Wall -O2 $(INC_DIRS)

# ARM flags
CFLAGS_ARM = -Wall -O3 \
             -march=armv8-a+simd \
             -ffast-math \
             -DFIXED_POINT -DARM_TARGET \
             $(INC_DIRS)

LDFLAGS_ARM = -static
LDLIBS_ARM  = -lm

# =========================
# HOST BUILD
# =========================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================
# ARM BUILD
# =========================
arm: $(TARGET_ARM)

$(TARGET_ARM): $(OBJS_ARM)
	$(CC_ARM) $(OBJS_ARM) -o $(TARGET_ARM) \
	    $(LDFLAGS_ARM) $(LDLIBS_ARM)

%.arm.o: %.c
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

# =========================
# TESTING (tests/ directory)
# =========================

TEST_DIR  = tests
BIN_DIR   = bin
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_SRCS))

# Core library source files needed for tests
FE_CORE_SRCS = src/fe_api.c src/module/dc_removal.c src/biquad/biquad.c

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Compile with host compiler
$(BIN_DIR)/%: $(TEST_DIR)/%.c $(FE_CORE_SRCS) | $(BIN_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< $(FE_CORE_SRCS) -o $@ $(LDLIBS)

# Compile with ARM compiler
$(BIN_DIR)/arm_%: $(TEST_DIR)/%.c $(FE_CORE_SRCS) | $(BIN_DIR)
	@echo "Compiling ARM $<..."
	@$(CC_ARM) $(CFLAGS_ARM) $< $(FE_CORE_SRCS) -o $@ $(LDFLAGS_ARM)

test_sincos: $(BIN_DIR)/test_sincos
	@echo "Running test_sincos..."
	@./$(BIN_DIR)/test_sincos

test_buffer_frame: $(BIN_DIR)/test_buffer_frame
	@echo "Running test_buffer_frame..."
	@./$(BIN_DIR)/test_buffer_frame

test_benchmark: $(BIN_DIR)/test_benchmark
	@echo "Running test_benchmark..."
	@./$(BIN_DIR)/test_benchmark

arm_test_benchmark: $(BIN_DIR)/arm_test_benchmark
	@echo "ARM test compiled: $(BIN_DIR)/arm_test_benchmark"

clean-test:
	@echo "Cleaning test artifacts..."
	@rm -rf $(BIN_DIR)
	
clean: clean-test
	rm -f $(OBJS) $(OBJS_ARM) $(TARGET) $(TARGET_ARM).elf

.PHONY: all arm_test_benchmark test test-all clean-test clean