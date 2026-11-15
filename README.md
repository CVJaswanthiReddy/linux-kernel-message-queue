# Multi-Client Message Queue Using a Linux Character Device Driver

## Overview

This project implements a Linux character device driver that provides a shared message queue through a circular buffer.

It demonstrates Linux kernel programming concepts, including character device registration, kernel memory management, synchronization, blocking and non-blocking I/O, wait queues, polling, and ioctl operations.

The driver allows multiple user-space processes to communicate through a shared kernel-space buffer.

**Project type:** Linux Kernel / Character Device Driver  
**Language:** C  
**Platform:** Linux (tested on Ubuntu 26.04.1 LTS, ARM64)  
**Kernel version tested:** 7.0.0-42-generic

## Features

- Linux character device registration using `cdev`.
- Circular buffer implementation with configurable capacity (4096 bytes).
- Read and write operations using `read()` and `write()`.
- Circular buffer wraparound handling.
- Blocking I/O using wait queues.
- Non-blocking I/O using `O_NONBLOCK`.
- Multiple concurrent readers and writers.
- Synchronization using kernel mutexes.
- Poll support for monitoring read and write readiness.
- Ioctl operations:
  - `MSGQUEUE_GET_COUNT` – retrieves the current number of queued bytes.
  - `MSGQUEUE_CLEAR` – clears the queue.
- User-space test programs for validating driver behaviour.

## Project Structure

```text
linux-kernel-message-queue/
│
├── driver/
│   ├── msgqueue.c
│   └── Makefile
│
├── user/
│   ├── writer.c
│   ├── reader.c
│   ├── open_close_test.c
│   ├── wraparound_test.c
│   ├── partial_copy_test.c
│   ├── blocking_write_test.c
│   ├── blocking_second_writer.c
│   ├── nonblocking_write_test.c
│   ├── fill_queue.c
│   ├── poll_test.c
│   ├── ioctl_test.c
│   └── ioctl_clear_test.c
│
└── msgqueue_ioctl.h
```

## Architecture

The driver uses a shared circular buffer in kernel space.

The main device structure maintains:

| Field | Purpose |
|---|---|
| `buffer` | Stores queued data |
| `capacity` | Maximum buffer capacity |
| `head` | Position from which data is read |
| `tail` | Position at which data is written |
| `count` | Number of bytes currently stored |
| `lock` | Protects shared queue state |
| `readq` | Wait queue for readers |
| `writeq` | Wait queue for writers |
| `cdev` | Character device interface |
| `devno` | Device number |

### Data flow

1. A user-space process opens `/dev/msgqueue`.
2. A writer sends data using `write()`.
3. The driver copies data into the circular buffer.
4. Readers retrieve data using `read()`.
5. The driver updates buffer positions and byte count.
6. Wait queues wake blocked processes when data or buffer space becomes available.

## Kernel Concepts Used

- **Character devices:** Registering a device and exposing file operations.
- **Circular buffers:** Managing data with head and tail positions.
- **Mutexes:** Protecting shared data against concurrent access.
- **Wait queues:** Supporting blocking read and write operations.
- **User-kernel data transfer:** Using `copy_to_user()` and `copy_from_user()`.
- **Non-blocking I/O:** Returning `EAGAIN` when an operation cannot proceed immediately.
- **Poll mechanism:** Implementing readiness notifications using `poll_wait()`.
- **Ioctl:** Supporting custom device control commands.
- **Kernel module lifecycle:** Implementing module initialization and cleanup.

## Requirements

- Linux system with kernel headers matching the running kernel.
- GCC compiler.
- GNU Make.
- Linux kernel development headers.
- Root or sudo privileges for loading the module and creating the device node.

On Ubuntu, the basic build dependencies can be installed using:

```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

## Build and Installation

### 1. Build the kernel module

```bash
cd driver
make
```

This generates `msgqueue.ko`.

### 2. Load the kernel module

```bash
sudo insmod msgqueue.ko
```

### 3. Find the registered major number

```bash
cat /proc/devices | grep msgqueue
```

Example output:

```text
235 msgqueue
```

The major number is dynamically allocated and may differ between systems.

### 4. Create the device node

The driver currently requires manual device-node creation.

Run:

```bash
MAJOR=$(awk '$2 == "msgqueue" {print $1}' /proc/devices)

sudo mknod /dev/msgqueue c "$MAJOR" 0
sudo chmod 666 /dev/msgqueue
```

### 5. Verify the device node

```bash
ls -l /dev/msgqueue
```

## Running User-Space Programs

Navigate to the user directory:

```bash
cd ~/ldd/linux-kernel-message-queue/user
```

Compile the basic writer and reader:

```bash
gcc -Wall -Wextra writer.c -o writer
gcc -Wall -Wextra reader.c -o reader
```

Run the writer:

```bash
./writer
```

Then run the reader:

```bash
./reader
```

### Compiling the test programs

For example:

```bash
gcc -Wall -Wextra open_close_test.c -o open_close_test
gcc -Wall -Wextra wraparound_test.c -o wraparound_test
gcc -Wall -Wextra partial_copy_test.c -o partial_copy_test
gcc -Wall -Wextra blocking_write_test.c -o blocking_write_test
gcc -Wall -Wextra nonblocking_write_test.c -o nonblocking_write_test
gcc -Wall -Wextra poll_test.c -o poll_test
gcc -Wall -Wextra ioctl_test.c -o ioctl_test
gcc -Wall -Wextra ioctl_clear_test.c -o ioctl_clear_test
```

Run each test according to its purpose. Some blocking and concurrent tests require multiple terminals or processes.

## Ioctl Commands

The project defines custom ioctl commands in `msgqueue_ioctl.h`.

| Command | Description |
|---|---|
| `MSGQUEUE_GET_COUNT` | Retrieves the number of bytes currently in the queue |
| `MSGQUEUE_CLEAR` | Resets the queue to an empty state |

The driver returns `ENOTTY` for unsupported ioctl commands.

## Testing and Validation

The following behaviours have been implemented and tested:

| Test | Result |
|---|---|
| Basic read and write | Passed |
| Circular buffer wraparound | Passed |
| Partial user-space copy handling | Passed |
| Blocking read | Passed |
| Blocking write | Passed |
| Non-blocking write | Passed |
| Multiple readers and writers | Passed |
| Concurrent producer-consumer operation | Passed |
| Poll readiness notification | Passed |
| Ioctl GET_COUNT | Passed |
| Ioctl CLEAR | Passed |
| Invalid ioctl command handling | Passed |

## Unloading the Module

When finished, unload the driver:

```bash
sudo rmmod msgqueue
```

Remove the manually created device node:

```bash
sudo rm -f /dev/msgqueue
```

Clean generated kernel build files:

```bash
cd ~/ldd/linux-kernel-message-queue/driver
make clean
```

## Current Limitations

- The implementation uses one shared queue for all open file descriptors.
- Data is handled as a byte stream; individual write boundaries are not preserved as separate messages.
- Device-node creation is manual and is not automated through a device class or udev rule.
- The current device node permissions allow all local users to read and write.

## Future Improvements

- Add automatic device-node creation using Linux device classes and udev.
- Add configurable buffer capacity.
- Explore separate queues for different clients.
- Add further stress and error-handling tests.
- Improve observability and kernel logging.

## Author

**Jaswanthi**

Linux Kernel Programming | Character Device Drivers | C Programming
