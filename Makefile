SRC_DIR = src
UTILS_DIR = utils
BIQUAD_DIR = src/biquad
ARM_CORTEX_M_DIR = arm-cortexM

TARGET = biquad_test
TARGET_ARM = biquad_test_arm

# Host compiler
CC = gcc

# ARM compiler
CC_ARM = arm-none-eabi-gcc

INC_DIRS = -Iinclude -I$(SRC_DIR) -I$(UTILS_DIR) -I$(BIQUAD_DIR)

LDLIBS = -lm
CFLAGS = -Wall -O2 -DFIXED_POINT $(INC_DIRS)

# ARM flags (Cortex-M3 bare-metal + QEMU)
CFLAGS_ARM = -Wall -g -O2 -DFIXED_POINT -DARM_TARGET $(INC_DIRS) \
             -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

LDFLAGS_ARM = -T arm-cortexM/linker.ld \
              -nostdlib -nostartfiles \
              -lgcc -lm

SRCS = src/main.c $(ARM_CORTEX_M_DIR)/startup.c

OBJS = $(SRCS:.c=.o)
OBJS_ARM = $(patsubst %.c,%.arm.o,$(SRCS))

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
	$(CC_ARM) $(OBJS_ARM) -o $(TARGET_ARM).elf $(LDFLAGS_ARM)

%.arm.o: %.c
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

# =========================
# TESTING (tests/ directory)
# =========================

TEST_DIR  = tests
BIN_DIR   = bin
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_SRCS))

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)


$(BIN_DIR)/%: $(TEST_DIR)/%.c | $(BIN_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

# Special rule for test_api to include fe_init.c
$(BIN_DIR)/test_api: $(TEST_DIR)/test_api.c src/fe_init.c | $(BIN_DIR)
	@echo "Compiling test_api.c with dependencies..."
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

test_sincos: $(BIN_DIR)/test_sincos
	@echo "Running test_sincos..."
	@./$(BIN_DIR)/test_sincos

test_dc: $(BIN_DIR)/test_dc_remov
	@echo "Running test_dc_remov..."
	@./$(BIN_DIR)/test_dc_remov

test_api: $(BIN_DIR)/test_api
	@echo "Running test_api..."
	@./$(BIN_DIR)/test_api

test-all: $(TEST_BINS)
	@echo "Running all tests..."
	@for bin in $(TEST_BINS); do \
		echo "---------------------------"; \
		echo "Executing $$bin..."; \
		./$$bin; \
	done
	@echo "---------------------------"

clean-test:
	@echo "Cleaning test artifacts..."
	@rm -rf $(BIN_DIR)
	
clean: clean-test
	rm -f $(OBJS) $(OBJS_ARM) $(TARGET) $(TARGET_ARM).elf

.PHONY: all arm test test-all clean-test clean