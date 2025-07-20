BUILD_DIR := build
INTERPRETER := result
EXE_DIR := $(BUILD_DIR)/release

CC:=gcc
CFLAGS:=-Wall -Wextra -O2

ifeq ($(MAKECMDGOALS), debug)
	CFLAGS += -g -DDEBUG
	INTERPRETER := $(INTERPRETER).dbg
	EXE_DIR := $(BUILD_DIR)/debug
endif

SRC:=$(wildcard *.c)
OBJS:=$(patsubst %.c, $(EXE_DIR)/%.o, $(SRC))

all debug: $(EXE_DIR)/$(INTERPRETER)

$(EXE_DIR)/$(INTERPRETER): $(OBJS)
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(EXE_DIR)/%.o: %.c
	mkdir -p $(EXE_DIR)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean debug