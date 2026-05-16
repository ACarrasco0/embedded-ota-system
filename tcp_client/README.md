# OTA TCP Client

This directory contains the PC-side OTA sender. It connects to the Raspberry TCP server and sends a firmware binary using the project frame protocol.

The client sends:

```text
OTA_START -> OTA_DATA... -> OTA_END
```

Each frame waits for an `ACK` from the target before the next frame is sent.

## Requirements

- Linux or WSL on the PC.
- `make`
- `gcc` for building the OTA client itself.
- ARM cross-compiler for building the Raspberry firmware from the PC.
- Network access to the Raspberry TCP server.

## One-command OTA

From this directory:

```sh
make run_ota
```

This does three things:

1. Builds `my_app` from the repository root using the configured ARM cross-compiler.
2. Checks that the firmware is ARM/aarch64 and not x86-64.
3. Builds and runs the OTA client to send the firmware to the Raspberry.

Default parameters are defined in `tcp_client/Makefile`:

```make
RASPBERRY_PI_IP = 192.168.0.24
SOCKET_PORT = 4050
FW_PATH = ../build/bin/my_app
APP_CROSS_COMPILE ?= arm-linux-gnueabihf-
```

Override them from the command line:

```sh
make run_ota RASPBERRY_PI_IP=192.168.0.24
make run_ota SOCKET_PORT=4050
make run_ota APP_CROSS_COMPILE=/path/to/toolchain/bin/arm-linux-gnueabihf-
make run_ota FW_PATH=../build/bin/my_app
```

For a 64-bit target toolchain:

```sh
make run_ota APP_CROSS_COMPILE=aarch64-linux-gnu-
```

## Manual Use

Build the client:

```sh
make
```

Send a firmware file:

```sh
./build/bin/ota_client --host 192.168.x.x --port 4050 --file ../build/bin/my_app
```

The `--port` argument is optional and defaults to `4050`:

```sh
./build/bin/ota_client --host 192.168.x.x --file build/my_app.arm
```

The old positional form is also supported:

```sh
./build/bin/ota_client 192.168.x.x 4050 ../build/bin/my_app
```

## Firmware Safety Check

Before sending OTA, always check the firmware:

```sh
file ../build/bin/my_app
```

Good examples:

```text
ELF 32-bit LSB executable, ARM, EABI5
ELF 64-bit LSB executable, ARM aarch64
```

Bad example:

```text
ELF 64-bit LSB pie executable, x86-64
```

Sending an x86-64 binary to the Raspberry will install a binary that systemd cannot execute.

## Useful Commands

Clean only the OTA client build:

```sh
make clean
```

Send a specific already-built firmware:

```sh
make run_ota FW_PATH=./my_app_rpi_v2
```

Change target IP:

```sh
make run_ota RASPBERRY_PI_IP=192.168.0.24
```

