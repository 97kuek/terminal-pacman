CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O2
BUILD_DIR := build
TARGET := $(BUILD_DIR)/terminal-pacman
SOURCES := src/main.c src/game.c src/render.c src/pathfind.c src/maze.c src/qghost.c

ifeq ($(OS),Windows_NT)
	SOURCES += src/platform_win.c
	TARGET := $(BUILD_DIR)/terminal-pacman.exe
else
	SOURCES += src/platform_unix.c
endif

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	$(TARGET)

test: | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/test_game.c src/game.c src/pathfind.c src/maze.c src/qghost.c -o $(BUILD_DIR)/test
	$(BUILD_DIR)/test

clean:
	rm -rf $(BUILD_DIR)
