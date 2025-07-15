BUILD_DIR:=build
INTERPRETER:=res.out

CC:=gcc
CFLAGS:=

SRC:=$(wildcard *.c)
OBJS:=$(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))

all: $(BUILD_DIR)/$(INTERPRETER)

debug: CFLAGS+=-g -DDEBUG
debug: all

$(BUILD_DIR)/$(INTERPRETER): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(BUILD_DIR)/*

.PHONY: all clean