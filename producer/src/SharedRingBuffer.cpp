#include "SharedRingBuffer.h"

namespace producer
{

void SharedRingBuffer::open(
	[[maybe_unused]] const std::string& name,
	[[maybe_unused]] const std::uint64_t ringBufferBytes,
	[[maybe_unused]] const std::size_t payloadSize)
{
	// TODO: shm_open + ftruncate + mmap; capacity = ringBufferBytes / slotSize.
}

void SharedRingBuffer::publish(
	[[maybe_unused]] const common::PacketHeader& header,
	[[maybe_unused]] const std::span<const std::uint8_t> payload)
{
	// TODO: seqlock-versioned write into the next slot, overwrite-oldest on
	// wraparound, FutexGate::wake() to notify a sleeping Consumer.
}

}	 // namespace producer
