CC = gcc

CFLAGS = -Wall -Wextra -Werror -g

RASPBERRY_PI_IP = 192.168.0.24
SOCKET_PORT = 4050
FW_PATH = ../build/bin/my_app

APP_CROSS_COMPILE ?= arm-linux-gnueabihf-

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

OUT_OTA = $(BIN_DIR)/ota_client

.PHONY: all clean build_fw verify_fw run_ota

all: $(OUT_OTA)

$(OUT_OTA): \
	$(OBJ_DIR)/ota_client_main.o \
	$(OBJ_DIR)/ota_client.o \
	$(OBJ_DIR)/tcp_client.o \
	$(OBJ_DIR)/tcp_frame_client.o
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $(OUT_OTA)

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I../src -c $< -o $@


# BUILD FIRMWARE
build_fw:
	$(MAKE) -C .. clean
	$(MAKE) -C .. CROSS_COMPILE=$(APP_CROSS_COMPILE)


# VERIFY FIRMWARE
verify_fw: build_fw
	file $(FW_PATH)
	file $(FW_PATH) | grep -E "ARM|aarch64" >/dev/null
	! file $(FW_PATH) | grep -q "x86-64"


# OTA FLOW
run_ota: verify_fw $(OUT_OTA)
	./$(OUT_OTA) $(RASPBERRY_PI_IP) $(SOCKET_PORT) $(FW_PATH)

clean:
	rm -rf $(BUILD_DIR)
