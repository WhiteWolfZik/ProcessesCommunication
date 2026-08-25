#include "FutexGate.h"

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <ctime>

namespace common
{

namespace
{

// Takes the futex word by reference and the timeout by value; the only raw
// pointers in this file are formed right here, at the point of passing them
// to the syscall, since that is the boundary the kernel API dictates.
std::int64_t futexSyscall(
	const std::atomic<std::uint32_t>& addr,
	const std::int32_t futexOp,
	const std::uint32_t val,
	const std::optional<std::chrono::milliseconds> timeout)
{
	std::optional<struct timespec> timeoutStorage;
	if (timeout.has_value())
	{
		const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(*timeout);
		const auto nanoseconds =
			std::chrono::duration_cast<std::chrono::nanoseconds>(*timeout - seconds);
		timeoutStorage = timespec{ seconds.count(), nanoseconds.count() };
	}

	return ::syscall(
		SYS_futex, &addr, futexOp, val, timeoutStorage ? &*timeoutStorage : nullptr, nullptr, 0);
}

}	 // namespace

FutexGate::FutexGate(const std::atomic<std::uint32_t>& futexWord)
	: futexWord_(futexWord)
{
}

void FutexGate::wait(
	const std::uint32_t expected, const std::optional<std::chrono::milliseconds> timeout) const
{
	// Ignoring the return value is intentional: EAGAIN (value already changed),
	// EINTR (signal delivered) and ETIMEDOUT are all handled by the caller
	// re-checking its own condition in a loop, per standard futex usage.
	futexSyscall(futexWord_, FUTEX_WAIT, expected, timeout);
}

void FutexGate::wake(const std::uint32_t count) const
{
	futexSyscall(futexWord_, FUTEX_WAKE, count, std::nullopt);
}

}	 // namespace common
