#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>

#include "FutexGate.h"
#include "PacketHeader.h"
#include "RingBufferLayout.h"

namespace producer
{

class SharedRingBuffer
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	SharedRingBuffer() = default;
	//! Destructor.
	~SharedRingBuffer();
	//! Copy constructor.
	SharedRingBuffer(const SharedRingBuffer&) = delete;
	//! Copy assignment operator.
	SharedRingBuffer& operator=(const SharedRingBuffer&) = delete;
	//! Move constructor.
	SharedRingBuffer(SharedRingBuffer&&) = delete;
	//! Move assignment operator.
	SharedRingBuffer& operator=(SharedRingBuffer&&) = delete;

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
	//! Name of the shared memory segment, used to shm_unlink it on destruction.
	std::string name_;
	//! Whole mapped region, including the control block; empty until open() succeeds.
	std::span<std::byte> mapping_;
	//! Control block, bound once open() maps the segment.
	std::optional<std::reference_wrapper<common::RingBufferControlBlock>> control_;
	//! Slot array, i.e. mapping_ after the control block.
	std::span<std::byte> slots_;
	//! Wakes a waiting Consumer; bound to control_->notify once open() maps the segment.
	std::optional<common::FutexGate> gate_;
};

}	 // namespace producer
