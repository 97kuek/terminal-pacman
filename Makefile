CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O2
BUILD_DIR := build
TARGET := $(BUILD_DIR)/terminal-pacman
SOURCES := src/main.c src/game.c src/render.c

ifeq ($(OS),Windows_NT)
	SOURCES += src/platform_win.c
	TARGET := $(BUILD_DIR)/terminal-pacman.exe
else
	SOURCES += src/platform_unix.c
endif

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

