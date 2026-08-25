#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>

namespace common
{

namespace futex
{

constexpr std::uint32_t defaultWakeCount{ 1U };

}	 // namespace futex

class FutexGate
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	explicit FutexGate(const std::atomic<std::uint32_t>& futexWord);

	//
	// Public interface.
	//
public:
	//! Blocks while futexWord == expected; std::nullopt waits indefinitely.
	void wait(
		const std::uint32_t expected,
		const std::optional<std::chrono::milliseconds> timeout = std::nullopt) const;
	//! Wakes up to count waiters blocked in wait().
	void wake(const std::uint32_t count = futex::defaultWakeCount) const;

	//
	// Private data members.
	//
private:
	//! Reference to the shared futex word; never null, never rebound. Only the
	//! address is ever used (the futex syscall reads/wakes it at the kernel
	//! level), so FutexGate itself never needs write access through the type.
	const std::atomic<std::uint32_t>& futexWord_;
};

}	 // namespace common
