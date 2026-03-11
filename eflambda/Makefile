TARGET_DIR := ./target
TARGET := $(TARGET_DIR)/lambda.exe

CC := gcc
CFLAGS_FILE := ./compile_flags.txt

$(TARGET): main.c lambda.h $(CFLAGS_FILE) | $(TARGET_DIR)
	$(CC) $< -o $@ `paste -sd " " $(CFLAGS_FILE)`

$(TARGET_DIR):
	mkdir -p $@

run: $(TARGET)
	./$<

clean:
	rm -r $(TARGET_DIR)

.PHONY: run clean
