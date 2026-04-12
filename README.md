## Project Objectives

The goal of this project is to design and develop a complete embedded Linux 
platform that simulates a real industrial device. The focus is on building a 
robust, modular and maintainable system that integrates low-level software,
operating system services and hardware interaction.

### Core Objectives

- Develop a modular application in C following embedded software design principles
- Understand and manage Linux processes, signals and system services using systemd
- Design a scalable architecture inspired by RTOS concepts (tasks, scheduling, state handling)
- Implement structured logging and error handling mechanisms

### System-Level Objectives

- Gain hands-on experience with Linux internals and interfaces (/proc, /sys, /dev)
- Build a minimal embedded Linux system using Buildroot
- Integrate the application into a custom root filesystem
- Understand system boot process and service initialization

### Hardware Integration

- Interface with hardware using GPIO, UART and I2C
- Simulate and later integrate real sensors and actuators
- Develop abstraction layers for hardware modules

### Communication

- Implement TCP/IP communication for remote control and monitoring
- Design a simple and extensible communication protocol
- Enable bidirectional data exchange between device and external systems

### Robustness and Reliability

- Implement watchdog mechanisms (system-level and application-level)
- Ensure graceful startup and shutdown using signal handling
- Design the system to recover from failures automatically

### Advanced Features

- Implement remote update capabilities (OTA) with basic rollback support
- Add dynamic configuration via files or remote commands
- Monitor system metrics such as CPU usage, memory and temperature

### Testing and Validation

- Develop automated test scripts using Python
- Execute remote tests via SSH
- Validate system behavior through logs and defined test cases

### Final Goal

Deliver a fully functional embedded Linux system, from application layer to 
system integration, demonstrating practical skills in embedded software
development, Linux systems, and hardware-software integration aligned with real 
industry requirements.