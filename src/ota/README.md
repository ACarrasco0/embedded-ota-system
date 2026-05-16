# OTA Module

This directory contains the OTA update implementation.

The OTA code is split into two layers:

- `ota.c`: OTA core logic.
- `ota_port.h` / `ota_port_posix.c`: system abstraction layer.

## OTA Core

`ota.c` owns the update state machine:

- `ota_start()`: validates the OTA header, opens the temporary firmware file, and initializes state.
- `ota_write_chunk()`: writes incoming firmware chunks and updates the running CRC32.
- `ota_stop()`: syncs and closes the file, validates size and CRC32, applies the update, and schedules restart.
- `ota_check_timeout()`: aborts stale OTA sessions.

The core does not call POSIX APIs directly. It only calls the `ota_port_*` interface, which makes the logic easier to mock in unit tests.

## Port Layer

`ota_port_posix.c` implements the system operations used by OTA:

- open/write/sync/close file
- remove/rename/chmod
- create parent directories
- time
- system command
- last error reporting

For unit tests, `tests/ota_port_mock.c` provides a fake in-memory implementation.

## Paths

The OTA flow uses:

```text
/opt/my_app/update/my_app.new
/opt/my_app/current/my_app
/opt/my_app/backup/my_app.bak
```

The current binary is moved to backup before the new binary is installed.

## Validation

Before applying an update, OTA checks:

- expected firmware size
- received firmware size
- CRC32 calculated over all chunks

If validation fails, the temporary file is removed and the current binary remains untouched.

## Important Notes

- OTA does not currently verify cryptographic signatures.
- The firmware must be compiled for the target architecture.
- A host PC x86-64 binary will not run on the Raspberry target.
- After successful update, the system currently schedules a reboot.
