SRC_DIR = src
UTILS_DIR = utils
BIQUAD_DIR = src/biquad

TARGET = biquad_test

CC = gcc

INC_DIRS = -Iinclude -I$(SRC_DIR) -I$(UTILS_DIR) -I$(BIQUAD_DIR)
LDLIBS = -lm
CFLAGS = -Wall -O3 -DFIXED_POINT $(INC_DIRS)

SRCS = src/main.c

OBJS = $(SRCS:.c=.o)

# build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean