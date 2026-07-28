SRC_DIR   = src
UTILS_DIR = utils
BIN_DIR   = bin

TARGET     = $(BIN_DIR)/test_rtafe
TARGET_ARM = $(BIN_DIR)/arm_test_rtafe

# =========================
# Source files for test_rtafe
# =========================
CPP_SRCS = tests/test_rtafe.cpp \
           $(SRC_DIR)/rtafe_main_sp.cpp \
           $(SRC_DIR)/rtafe_main_ap.cpp \
           $(SRC_DIR)/buffer/buffer_mng.cpp \
           $(SRC_DIR)/buffer/wav_file_mgr.cpp \
           $(SRC_DIR)/biquad/biquad_filter.cpp \
           $(SRC_DIR)/biquad/i_biquad_design.cpp \
           $(SRC_DIR)/module/noise_suppress.cpp \
           $(SRC_DIR)/module/dc_removal.cpp \
           $(SRC_DIR)/module/pre_emphasis.cpp \
           $(SRC_DIR)/module/idsp_module.cpp

C_SRCS = $(SRC_DIR)/module/fft.c

INC_DIRS = -I$(SRC_DIR) -I$(UTILS_DIR) -I$(SRC_DIR)/biquad -I$(SRC_DIR)/module -I$(SRC_DIR)/buffer -I$(SRC_DIR)/api

# =========================
# Host compiler (x86)
# =========================
CXX = g++
CC  = gcc

CXXFLAGS = -Wall -O2 -std=c++14 -DFIXED_POINT $(INC_DIRS)
CFLAGS   = -Wall -O2 -DFIXED_POINT $(INC_DIRS)
LDFLAGS  = -lm -lstdc++

# =========================
# ARM compiler (AArch64)
# =========================
CXX_ARM = aarch64-linux-gnu-g++
CC_ARM  = aarch64-linux-gnu-gcc

CXXFLAGS_ARM = -Wall -O2 -std=c++14 \
               -march=armv8-a+simd -ffast-math \
               -DFIXED_POINT -DARM_TARGET $(INC_DIRS)
CFLAGS_ARM   = -Wall -O2 -march=armv8-a+simd -ffast-math \
               -DFIXED_POINT -DARM_TARGET $(INC_DIRS)
LDFLAGS_ARM  = -static -lm -lstdc++

# =========================
# Object files
# =========================
OBJS     = $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
OBJS_ARM = $(CPP_SRCS:.cpp=.arm.o) $(C_SRCS:.c=.arm.o)

# =========================
# HOST BUILD (default)
# =========================
all: dirs $(TARGET)

dirs:
	@mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# =========================
# ARM BUILD
# =========================
arm: dirs $(TARGET_ARM)

$(TARGET_ARM): $(OBJS_ARM)
	$(CXX_ARM) $(OBJS_ARM) -o $(TARGET_ARM) $(LDFLAGS_ARM)

%.arm.o: %.cpp
	$(CXX_ARM) $(CXXFLAGS_ARM) -c $< -o $@

%.arm.o: %.c
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

# =========================
# RUN
# =========================
run: $(TARGET)
	@./$(TARGET)

run-wav: $(TARGET)
	@if [ -z "$(INPUT)" ] || [ -z "$(OUTPUT)" ]; then \
		echo "Usage: make run-wav INPUT=input.wav OUTPUT=output.wav"; \
		exit 1; \
	fi
	@./$(TARGET) $(INPUT) $(OUTPUT)

# =========================
# CLEAN
# =========================
clean:
	rm -f $(OBJS) $(OBJS_ARM) $(TARGET) $(TARGET_ARM)

.PHONY: all arm dirs clean run run-wav