#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "PacketHeader.h"

namespace consumer
{

//! One packet read out of the ring buffer: header plus a owned copy of the payload.
struct Packet
{
	//! Packet metadata.
	common::PacketHeader header;
	//! Payload bytes, sized to header.payloadSize.
	std::vector<std::uint8_t> payload;
};

//! Attaches to the shared-memory segment created by Producer. Implementation
//! lands in a dedicated stage; this is currently a skeleton (declarations only).
class SharedRingBufferReader
{
	//
	// Public interface.
	//
public:
	//! Attaches to the named segment, retrying with backoff until it exists.
	void attach(const std::string& name);
	//! Returns the next packet if available, without blocking indefinitely.
	std::optional<Packet> tryConsume();

	//
	// Private data members.
	//
private:
	//! File descriptor of the shared memory segment.
	std::int32_t shmFd_{ -1 };
	//! Mapped slot array, reassigned once attach() maps the segment.
	std::span<std::byte> slots_;
	//! Number of slots in the ring buffer.
	std::size_t capacity_{ 0 };
	//! Shared write index, bound once attach() maps the segment.
	std::optional<std::reference_wrapper<std::atomic<std::uint64_t>>> writeIndex_;
	//! Shared read index, bound once attach() maps the segment.
	std::optional<std::reference_wrapper<std::atomic<std::uint64_t>>> readIndex_;
};

}	 // namespace consumer
