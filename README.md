# Embedded Linux OTA Platform

This project is a small embedded Linux system written in C. It implements a TCP server, a binary frame protocol, and an OTA update flow able to receive a new application binary, validate it, swap it into place, and restart the target system.

The project was built as a learning and portfolio exercise around embedded Linux, TCP communication, protocol framing, OTA update flows, and system abstraction in C. It is not intended to be an industrial-grade updater or a production safety mechanism.

## Main Features

- TCP server running on the Raspberry Pi target.
- Binary frame protocol with sequence number, command byte, escaping, and CRC.
- OTA update system with `START`, `DATA`, and `END` flow.
- Firmware size and CRC32 validation before applying an update.
- Port layer abstraction for filesystem and OS calls.
- Basic unit tests for frame, protocol, and OTA logic.
- PC-side OTA client used to send firmware updates to the device.

## Architecture

```text
PC (builder)
  |
  v
OTA TCP client
  |
  v
Raspberry Pi (runtime)
  |
  v
TCP server -> protocol -> OTA -> filesystem -> binary swap -> restart
```

The Raspberry runs `my_app` as a systemd service. The PC builds or selects a target-compatible firmware binary and sends it over TCP using the OTA client in `tcp_client/`.

## Requirements

- `make`
- `gcc` for local/native builds
- ARM cross-compiler for PC-side Raspberry builds, for example `arm-linux-gnueabihf-gcc`
- Linux or WSL when building/sending from a PC
- Raspberry Pi or embedded Linux target with systemd
- TCP connectivity from the PC to the target

## Project Structure

```text
src/
  comms/        TCP server, frame codec, protocol dispatch
  ota/          OTA core logic and OTA port layer
  platform/     System startup/shutdown hooks
  logger/       Simple logging helpers
  core/         Scheduler and shared config
tests/          Unit tests and mocks
tcp_client/     OTA sender used from the PC
systemd/        systemd service file for the target
```

## Build Locally

From the repository root:

```sh
make
```

For a local development run:

```sh
make run-dev
```

`run-dev` stops the system service first and then runs the locally built binary from `build/bin/my_app`.

## Run As A Service

The service file is stored in:

```text
systemd/my_app.service
```

Copy it to the Raspberry:

```sh
sudo cp systemd/my_app.service /etc/systemd/system/my_app.service
sudo systemctl daemon-reload
sudo systemctl enable my_app
sudo systemctl start my_app
```

Follow logs with:

```sh
journalctl -u my_app -f
```

## Install On The Raspberry

If you are building directly on the Raspberry, use:

```sh
make clean
make
sudo make install
```

This installs the binary to:

```text
/opt/my_app/current/my_app
```

and restarts the service.

## OTA From A PC

The normal OTA workflow is launched from the PC using the client Makefile:

```sh
cd tcp_client
make run_ota
```

This target:

1. Builds `my_app` from the repository root using the configured ARM cross-compiler.
2. Verifies that the generated firmware is ARM/aarch64 and not x86-64.
3. Builds the OTA client for the PC.
4. Sends the firmware to the Raspberry over TCP.

Default OTA parameters are configured in `tcp_client/Makefile`:

```make
RASPBERRY_PI_IP = 192.168.0.24
SOCKET_PORT = 4050
FW_PATH = ../build/bin/my_app
APP_CROSS_COMPILE ?= arm-linux-gnueabihf-
```

Override them from the command line:

```sh
make run_ota RASPBERRY_PI_IP=192.168.0.24 SOCKET_PORT=4050
make run_ota APP_CROSS_COMPILE=/path/to/toolchain/bin/arm-linux-gnueabihf-
make run_ota FW_PATH=../build/bin/my_app
```

Manual OTA is also supported:

```sh
./tcp_client/build/bin/ota_client --host 192.168.x.x --file build/my_app.arm
```

or with an explicit port:

```sh
./tcp_client/build/bin/ota_client --host 192.168.x.x --port 4050 --file build/my_app.arm
```

## Important Notes

- OTA firmware must be built for the Raspberry target architecture.
- Do not deploy a binary built with the host PC `gcc` unless the target is the same architecture.
- Always check firmware before sending:

```sh
file build/bin/my_app
```

Good target examples:

```text
ELF 32-bit LSB executable, ARM, EABI5
ELF 64-bit LSB executable, ARM aarch64
```

Bad deployment example:

```text
ELF 64-bit LSB pie executable, x86-64
```

- The OTA core validates both expected size and CRC32 before applying the update.
- The current binary is moved to `/opt/my_app/backup/my_app.bak` before the new binary is installed.
- After a successful OTA, the system currently schedules a reboot so the updated binary starts cleanly.

## Tests

Run tests from the repository root:

```sh
make test
```

The most important tests cover:

- frame encode/decode and CRC behavior
- protocol command dispatch and ACK/NACK responses
- OTA core validation using a mocked port layer

## Current Scope

This project is intentionally compact. The focus is on understanding the moving parts of an embedded update path:

- TCP transport
- robust frame parsing
- command protocol
- OTA state machine
- filesystem swap
- testability through mockable system calls

It is a portfolio and learning project, not a complete secure OTA product.
