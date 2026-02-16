# Makefile for cedit

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -Igapbuffer/src

# Directories
SRC_DIR = src
GB_DIR = gapbuffer/src
BUILD_DIR = build

# All .c files
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(GB_DIR)/*.c)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Executable
TARGET = editor

# Default rule
all: $(BUILD_DIR) $(TARGET)

# Build executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compile .c -> .o
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
