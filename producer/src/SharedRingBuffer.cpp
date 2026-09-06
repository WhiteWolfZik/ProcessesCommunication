#include "SharedRingBuffer.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

namespace producer
{

namespace
{

constexpr auto segmentPermissions{ 0600 };

[[noreturn]] void failWithErrno(const char* const what)
{
	std::fprintf(stderr, "%s: %s\n", what, std::strerror(errno));
	std::exit(EXIT_FAILURE);
}

[[noreturn]] void fail(const char* const what)
{
	std::fprintf(stderr, "%s\n", what);
	std::exit(EXIT_FAILURE);
}

std::size_t pageAlignedSize(const std::size_t size)
{
	const auto pageSize{ static_cast<std::size_t>(::sysconf(_SC_PAGESIZE)) };
	return common::ringBufferLayout::alignUp(size, pageSize);
}

}	 // namespace

SharedRingBuffer::~SharedRingBuffer()
{
	if (!mapping_.empty())
	{
		::munmap(mapping_.data(), mapping_.size());
	}
	if (!name_.empty())
	{
		::shm_unlink(name_.c_str());
	}
}

void SharedRingBuffer::open(
	const std::string& name, const std::uint64_t ringBufferBytes, const std::size_t payloadSize)
{
	const auto capacity{ common::ringBufferLayout::capacityFor(ringBufferBytes, payloadSize) };
	if (capacity == 0)
	{
		fail("ring buffer too small for even one slot");
	}
	const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
	const auto totalSize{ pageAlignedSize(
		sizeof(common::RingBufferControlBlock) + capacity * stride) };

	std::int32_t fd{ ::shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, segmentPermissions) };
	if (fd < 0 && errno == EEXIST)
	{
		// Leftover from a previous crashed run: treat it as stale and recreate.
		::shm_unlink(name.c_str());
		fd = ::shm_open(name.c_str(), O_CREAT | O_EXCL | O_RDWR, segmentPermissions);
	}
	if (fd < 0)
	{
		failWithErrno("shm_open");
	}

	if (::ftruncate(fd, static_cast<off_t>(totalSize)) != 0)
	{
		failWithErrno("ftruncate");
	}

	void* const mapped{ ::mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) };
	::close(fd);
	if (mapped == MAP_FAILED)
	{
		failWithErrno("mmap");
	}

	mapping_ = std::span(static_cast<std::byte*>(mapped), totalSize);

	auto& control{ *new (mapping_.data()) common::RingBufferControlBlock{} };
	control.capacity = capacity;
	control.payloadSize = static_cast<std::uint32_t>(payloadSize);

	control_ = control;
	slots_ = mapping_.subspan(sizeof(common::RingBufferControlBlock));
	gate_.emplace(control.notify);
	name_ = name;
}

void SharedRingBuffer::publish(
	const common::PacketHeader& header, const std::span<const std::uint8_t> payload)
{
	auto& control{ control_->get() };
	if (payload.size() != control.payloadSize)
	{
		fail("publish: payload size does not match the size configured at open()");
	}

	const auto stride{ common::ringBufferLayout::slotStride(control.payloadSize) };
	const auto writeIndexValue{ control.writeIndex.load(std::memory_order_relaxed) };
	const auto index{ writeIndexValue % control.capacity };
	std::byte* const slot{ slots_.data() + common::ringBufferLayout::slotOffset(index, stride) };

	auto& slotHeader{ *reinterpret_cast<common::SlotHeader*>(slot) };
	auto* const packetHeader{ reinterpret_cast<common::PacketHeader*>(
		slot + sizeof(common::SlotHeader)) };
	std::byte* const packetPayload{ slot + sizeof(common::SlotHeader)
									+ sizeof(common::PacketHeader) };

	const auto wasEmpty{ writeIndexValue == control.readIndex.load(std::memory_order_relaxed) };

	slotHeader.version.fetch_add(1, std::memory_order_relaxed);
	slotHeader.stampedIndex.store(writeIndexValue, std::memory_order_relaxed);
	*packetHeader = header;
	std::memcpy(packetPayload, payload.data(), payload.size());
	slotHeader.version.fetch_add(1, std::memory_order_release);

	control.writeIndex.fetch_add(1, std::memory_order_release);
	control.notify.fetch_add(1, std::memory_order_relaxed);
	if (wasEmpty)
	{
		gate_->wake();
	}
}

}	 // namespace producer
