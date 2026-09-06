#include "SharedRingBufferReader.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "SignalController.h"

namespace consumer
{

namespace
{

constexpr auto attachRetryInterval{ std::chrono::milliseconds(100) };
constexpr auto pollTimeout{ std::chrono::milliseconds(100) };

[[noreturn]] void failWithErrno(const char* const what)
{
	std::fprintf(stderr, "%s: %s\n", what, std::strerror(errno));
	std::exit(EXIT_FAILURE);
}

}	 // namespace

SharedRingBufferReader::~SharedRingBufferReader()
{
	if (!mapping_.empty())
	{
		::munmap(mapping_.data(), mapping_.size());
	}
}

bool SharedRingBufferReader::attach(
	const std::string& name, const common::SignalController& signals)
{
	std::int32_t fd{ -1 };
	for (;;)
	{
		fd = ::shm_open(name.c_str(), O_RDWR, 0);
		if (fd >= 0)
		{
			break;
		}
		if (errno != ENOENT)
		{
			failWithErrno("shm_open");
		}
		if (signals.isStopRequested())
		{
			return false;
		}
		std::this_thread::sleep_for(attachRetryInterval);
	}

	struct stat segmentStat
	{
	};
	if (::fstat(fd, &segmentStat) != 0)
	{
		failWithErrno("fstat");
	}
	const auto totalSize{ static_cast<std::size_t>(segmentStat.st_size) };

	void* const mapped{ ::mmap(nullptr, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) };
	::close(fd);
	if (mapped == MAP_FAILED)
	{
		failWithErrno("mmap");
	}

	mapping_ = std::span(static_cast<std::byte*>(mapped), totalSize);
	auto& control{ *reinterpret_cast<common::RingBufferControlBlock*>(mapping_.data()) };
	control_ = control;
	slots_ = std::span<const std::byte>(mapping_).subspan(sizeof(common::RingBufferControlBlock));
	gate_.emplace(control.notify);
	return true;
}

bool SharedRingBufferReader::tryConsume(Packet& out)
{
	auto& control{ control_->get() };

	const auto writeIndex{ control.writeIndex.load(std::memory_order_acquire) };
	auto readIndex{ control.readIndex.load(std::memory_order_relaxed) };

	if (writeIndex - readIndex > control.capacity)
	{
		readIndex = writeIndex - control.capacity;
	}

	if (readIndex == writeIndex)
	{
		control.readIndex.store(readIndex, std::memory_order_relaxed);
		const auto notifyValue{ control.notify.load(std::memory_order_acquire) };
		gate_->wait(notifyValue, pollTimeout);
		return false;
	}

	const auto stride{ common::ringBufferLayout::slotStride(control.payloadSize) };
	const auto index{ readIndex % control.capacity };
	const std::byte* const slot{ slots_.data()
								 + common::ringBufferLayout::slotOffset(index, stride) };

	const auto& slotHeader{ *reinterpret_cast<const common::SlotHeader*>(slot) };
	const auto* const packetHeader{ reinterpret_cast<const common::PacketHeader*>(
		slot + sizeof(common::SlotHeader)) };
	const std::byte* const packetPayload{ slot + sizeof(common::SlotHeader)
										  + sizeof(common::PacketHeader) };

	const auto peekStamped{ slotHeader.stampedIndex.load(std::memory_order_acquire) };
	if (peekStamped < readIndex)
	{
		control.readIndex.store(readIndex, std::memory_order_relaxed);
		return false;
	}
	if (peekStamped > readIndex)
	{
		control.readIndex.store(readIndex + 1, std::memory_order_relaxed);
		return false;
	}

	out.payload.resize(control.payloadSize);
	std::uint64_t stampedIndex{ peekStamped };
	for (;;)
	{
		const auto versionBefore{ slotHeader.version.load(std::memory_order_acquire) };
		if (versionBefore % 2 != 0)
		{
			continue;
		}

		stampedIndex = slotHeader.stampedIndex.load(std::memory_order_acquire);
		out.header = *packetHeader;
		std::memcpy(out.payload.data(), packetPayload, control.payloadSize);

		const auto versionAfter{ slotHeader.version.load(std::memory_order_acquire) };
		if (versionBefore == versionAfter)
		{
			break;
		}
	}

	if (stampedIndex != readIndex)
	{
		control.readIndex.store(readIndex + 1, std::memory_order_relaxed);
		return false;
	}

	control.readIndex.store(readIndex + 1, std::memory_order_relaxed);
	return true;
}

}	 // namespace consumer
