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
clean:
	rm -f $(OBJS) $(OBJS_ARM) $(TARGET) $(TARGET_ARM).elf

.PHONY: all arm clean