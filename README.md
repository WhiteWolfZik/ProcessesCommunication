# ProcessesCommunication

Two independent POSIX CLI applications communicating over a lock-free ring buffer in shared memory — no message queues, sockets, or third-party IPC libraries.

- **Producer** — generates packets of random bytes at a configurable payload size, stamps each with a sequence number, timestamp and checksum, and publishes them into a shared-memory ring buffer as fast as possible.
- **Consumer** — attaches to that ring buffer, validates each packet's checksum and sequence continuity, and periodically reports throughput (packets/sec, bytes/sec).

Both processes can be paused and resumed independently via POSIX signals (`SIGUSR1`/`SIGUSR2`) or a keypress, without losing synchronization with each other.

## Design

See [`doc/description.docx`](doc/description.docx) for the system overview and the rationale behind choosing shared memory. Per-application design (class responsibilities + class/sequence diagrams) lives under [`doc/producer`](doc/producer) and [`doc/consumer`](doc/consumer).
