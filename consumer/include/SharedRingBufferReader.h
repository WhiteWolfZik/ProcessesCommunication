#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "FutexGate.h"
#include "PacketHeader.h"
#include "RingBufferLayout.h"

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

//! Attaches to the shared-memory segment created by Producer and reads
//! packets out of it. Never creates or unlinks the segment — that is
//! Producer's responsibility.
class SharedRingBufferReader
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	SharedRingBufferReader() = default;
	//! Destructor.
	~SharedRingBufferReader();
	//! Copy constructor.
	SharedRingBufferReader(const SharedRingBufferReader&) = delete;
	//! Copy assignment operator.
	SharedRingBufferReader& operator=(const SharedRingBufferReader&) = delete;
	//! Move constructor.
	SharedRingBufferReader(SharedRingBufferReader&&) = delete;
	//! Move assignment operator.
	SharedRingBufferReader& operator=(SharedRingBufferReader&&) = delete;

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
	//! Whole mapped region, including the control block; empty until attach() succeeds.
	std::span<std::byte> mapping_;
	//! Control block, bound once attach() maps the segment.
	std::optional<std::reference_wrapper<common::RingBufferControlBlock>> control_;
	//! Slot array, i.e. mapping_ after the control block. Read-only from this
	//! side: Consumer only ever copies slot bytes out, never writes into them.
	std::span<const std::byte> slots_;
	//! Waits for Producer to publish; bound to control_->notify once attach() maps the segment.
	std::optional<common::FutexGate> gate_;
};

}	 // namespace consumer
