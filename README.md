# ProcessesCommunication

Two independent POSIX CLI applications communicating over a lock-free ring buffer in shared memory — no message queues, sockets, or third-party IPC libraries.

- **Producer** — generates packets of random bytes at a configurable payload size, stamps each with a sequence number, timestamp and checksum, and publishes them into a shared-memory ring buffer as fast as possible.
- **Consumer** — attaches to that ring buffer, validates each packet's checksum and sequence continuity, and periodically reports throughput (packets/sec, bytes/sec).

Both processes can be paused and resumed independently via POSIX signals (`SIGUSR1`/`SIGUSR2`) or a keypress, without losing synchronization with each other.

## Build & run

Requires CMake 3.16+, a C++20 compiler, and network access on the first configure (unit tests are fetched via `FetchContent`; pass `-DBUILD_TESTING=OFF` to skip that entirely).

**Via CMake directly:**
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./build/producer/producer <payloadSize> [--buffer-size BYTES]
./build/consumer/consumer [--interval SECONDS]
```

**Via the Makefile wrapper** (same build underneath, plus convenience run targets):
```sh
make start-producer   # PAYLOAD_SIZE=256 by default; override with PAYLOAD_SIZE=... BUFFER_SIZE=...
make start-consumer   # INTERVAL=1 by default; override with INTERVAL=...
make start-all        # runs both together (producer in the background), Ctrl-C stops both
make test             # builds and runs the unit test suite via ctest
make clean
```

Run producer and consumer in separate terminals (order doesn't matter — Consumer retries until Producer creates the segment). While either is running: `Ctrl-C` shuts it down gracefully; `SIGUSR1`/`SIGUSR2` (or any keypress in that terminal) pause/resume it.

## Design

See [`doc/description.docx`](doc/description.docx) for the system overview and the rationale behind choosing shared memory. Per-application design (class responsibilities + class/sequence diagrams) lives under [`doc/producer`](doc/producer) and [`doc/consumer`](doc/consumer).
