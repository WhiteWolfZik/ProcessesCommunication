#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "PacketHeader.h"

namespace common
{

//! Fixed-size header at the start of the mapped segment, shared by both processes.
struct RingBufferControlBlock
{
	//! Slot index Producer will write to next. Cache-line-separated from
	//! readIndex to avoid false sharing between the two processes' cores.
	alignas(64) std::atomic<std::uint64_t> writeIndex{ 0 };
	//! Slot index Consumer will read from next.
	alignas(64) std::atomic<std::uint64_t> readIndex{ 0 };
	//! Futex word Consumer waits on; bumped by Producer on every publish.
	//! Deliberately separate from writeIndex: futex() only operates on a
	//! 32-bit word, but writeIndex is 64-bit.
	std::atomic<std::uint32_t> notify{ 0 };
	//! Number of slots, set once by Producer at creation.
	std::uint64_t capacity{ 0 };
	//! Payload size in bytes, set once by Producer at creation.
	std::uint32_t payloadSize{ 0 };
};

//! Per-slot seqlock guard, immediately followed in memory by a PacketHeader
//! and then payloadSize bytes of payload. Even version = stable; odd = a
//! write is in progress. alignas(8) keeps the PacketHeader that follows it
//! 8-byte aligned across every slot.
struct alignas(8) SlotHeader
{
	std::atomic<std::uint32_t> version{ 0 };
};

namespace ringBufferLayout
{

constexpr std::size_t alignmentBytes{ 8 };

//! Rounds value up to the next multiple of alignment.
constexpr std::size_t alignUp(const std::size_t value, const std::size_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

//! Byte distance from one slot's start to the next.
constexpr std::size_t slotStride(const std::size_t payloadSize)
{
	return alignUp(sizeof(SlotHeader) + sizeof(PacketHeader) + payloadSize, alignmentBytes);
}

//! Byte offset of slot `index` from the start of the slot array (i.e. from
//! immediately after the RingBufferControlBlock).
constexpr std::size_t slotOffset(const std::uint64_t index, const std::size_t stride)
{
	return static_cast<std::size_t>(index) * stride;
}

//! Number of slots that fit in ringBufferBytes after the control block.
constexpr std::uint64_t capacityFor(
	const std::uint64_t ringBufferBytes, const std::size_t payloadSize)
{
	const std::size_t stride{ slotStride(payloadSize) };
	if (stride == 0 || ringBufferBytes <= sizeof(RingBufferControlBlock))
	{
		return 0;
	}
	return (ringBufferBytes - sizeof(RingBufferControlBlock)) / stride;
}

}	 // namespace ringBufferLayout

}	 // namespace common
