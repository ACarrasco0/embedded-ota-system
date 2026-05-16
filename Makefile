# Makefile for building the application
# DEV:  build/bin/my_app
# PROD: /opt/my_app/current/my_app

CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc

CFLAGS = -Wall -Wextra -Werror -g -Isrc

SRC = $(shell find src -type f -name "*.c")
OBJ = $(patsubst src/%.c, build/obj/%.o, $(SRC))

OUT = build/bin/my_app

.PHONY: all clean run run-dev run-prod install test ota-sim


# BUILD

all: $(OUT)

$(OUT): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

build/obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# DEV RUN 

run-dev: $(OUT)
	@echo "[DEV] Stopping system service..."
	-sudo systemctl stop my_app

	@sleep 1

	@echo "[DEV] Running local binary"
	./$(OUT)

# backward compatibility
run: run-dev


# PROD RUN

run-prod:
	@echo "[PROD] Running installed binary"
	sudo /opt/my_app/current/my_app


# TESTS

test: tests/tcp_frame_server_test tests/ota_tests tests/tcp_protocol_server_test
	./tests/tcp_frame_server_test
	./tests/ota_tests
	./tests/tcp_protocol_server_test

tests/tcp_frame_server_test: tests/tcp_frame_server_test.c src/comms/tcp_frame_server.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I. $< src/comms/tcp_frame_server.c -o $@

tests/ota_tests: tests/ota_tests.c tests/ota_port_mock.c src/ota/ota.c src/logger/logger.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I. $< tests/ota_port_mock.c src/ota/ota.c src/logger/logger.c -o $@

tests/tcp_protocol_server_test: tests/tcp_protocol_server_test.c tests/tcp_server_mock.c src/comms/tcp_protocol_server.c src/comms/tcp_frame_server.c src/logger/logger.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I. $< tests/tcp_server_mock.c src/comms/tcp_protocol_server.c src/comms/tcp_frame_server.c src/logger/logger.c -o $@

 
# CLEAN

clean:
	rm -rf build


# INSTALL (PRODUCTION DEPLOY)

install: $(OUT)
	@echo "[PROD] Installing binary"
	sudo systemctl stop my_app || true

	sudo mkdir -p /opt/my_app/current
	sudo mkdir -p /opt/my_app/update
	sudo mkdir -p /opt/my_app/backup

	sudo cp $(OUT) /opt/my_app/current/my_app
	sudo chmod +x /opt/my_app/current/my_app

	sudo systemctl daemon-reload
	sudo systemctl restart my_app


# OTA SIMULATION (DEV TOOL)

ota-sim: $(OUT)
	@echo "[OTA-SIM] Simulating OTA deployment..."

	sudo mkdir -p /opt/my_app/update
	sudo mkdir -p /opt/my_app/current

	# simulate OTA upload target
	sudo cp $(OUT) /opt/my_app/update/my_app.new

	# simulate validation + swap
	sudo mv /opt/my_app/current/my_app /opt/my_app/backup/my_app.bak || true
	sudo mv /opt/my_app/update/my_app.new /opt/my_app/current/my_app

	sudo chmod +x /opt/my_app/current/my_app

	@echo "[OTA-SIM] Restarting service"
	sudo systemctl restart my_app || true
