# Makefile for building the application
# Use "make" to build the application
# Use "make run" to build and run the application

CC = gcc

CFLAGS = -Wall -Wextra -Werror -Iinclude

SRC = $(shell find src -name "*.c")
OBJ = $(patsubst src/%.c, build/obj/%.o, $(SRC))

OUT = build/bin/my_app

.PHONY: all clean run

all: $(OUT)

$(OUT): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

build/obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(OUT)
	./$(OUT)

clean:
	rm -rf build

# Install the application to the system by stopping the service if it's running,
# copy the binary to /usr/local/bin, and restart the service. It is necessary to
# reload systemd to recognize the new binary.
install:
	sudo systemctl stop my_app || true
	sudo cp build/bin/my_app /usr/local/bin/my_app
	sudo chmod +x /usr/local/bin/my_app
	sudo systemctl daemon-reload
	sudo systemctl restart my_app
