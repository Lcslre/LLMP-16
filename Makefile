CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

SRC_DIR = .
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

DEPS = llmp16.h llmp16_dma.h llmp16_PIC.h BIOS_FONT.h
TARGET = main

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
