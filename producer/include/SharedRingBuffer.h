#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>

#include "PacketHeader.h"

namespace producer
{

class SharedRingBuffer
{
	//
	// Public interface.
	//
public:
	//! Creates and maps the named shared memory segment sized from ringBufferBytes.
	void open(
		const std::string& name,
		const std::uint64_t ringBufferBytes,
		const std::size_t payloadSize);
	//! Publishes one packet into the ring buffer.
	void publish(const common::PacketHeader& header, const std::span<const std::uint8_t> payload);

	//
	// Private data members.
	//
private:
	//! File descriptor of the shared memory segment.
	std::int32_t shmFd_{ -1 };
	//! Mapped slot array, reassigned once open() maps the segment.
	std::span<std::byte> slots_;
	//! Number of slots, derived from ringBufferBytes / slotSize.
	std::size_t capacity_{ 0 };
	//! Shared write index, bound once open() maps the segment.
	std::optional<std::reference_wrapper<std::atomic<std::uint64_t>>> writeIndex_;
	//! Shared read index, bound once open() maps the segment.
	std::optional<std::reference_wrapper<std::atomic<std::uint64_t>>> readIndex_;
};

}	 // namespace producer
