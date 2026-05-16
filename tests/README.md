# Tests

This directory contains small unit tests for the core protocol and OTA logic.

The tests are intentionally simple and use `assert()` plus local mocks instead of an external test framework.

## Test Files

- `tcp_frame_server_test.c`: validates frame encode/decode, escaping, CRC handling, incomplete frames, invalid frames, and multiple frames.
- `tcp_protocol_server_test.c`: validates command dispatch and ACK/NACK behavior using a mocked TCP send function and fake OTA operations.
- `ota_tests.c`: validates OTA start/write/stop behavior, size checks, CRC checks, timeout handling, and error paths using a mocked OTA port layer.
- `ota_port_mock.c`: in-memory mock implementation of the OTA port layer.
- `tcp_server_mock.c`: mock implementation of `tcp_server_send()` for protocol tests.

`tcp_server_task_test.c` is experimental and should be treated with caution compared with the frame/protocol/OTA tests.

## Running Tests

From the repository root:

```sh
make test
```

The tests are focused on the logic that matters most for this project:

- TCP frame reliability
- protocol command routing
- OTA validation and state transitions
- mockability of system operations
