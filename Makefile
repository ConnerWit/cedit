# Makefile for cedit

CC = gcc

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

INCLUDES = -I$(INC_DIR)

CFLAGS = -Wall -Wextra -g $(INCLUDES) -MMD -MP

SRCS := $(wildcard $(SRC_DIR)/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

DEPS := $(OBJS:.o=.d)

TARGET = $(BUILD_DIR)/ceditor

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)

.PHONY: all clean
