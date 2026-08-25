#pragma once

#include <cstddef>
#include <cstdint>

namespace producer
{

namespace defaults
{

constexpr std::uint64_t bytesPerMebibyte{ 1024ULL * 1024ULL };
constexpr std::size_t payloadSize{ 64 };
constexpr std::uint64_t ringBufferBytes{ 64 * bytesPerMebibyte };

}	 // namespace defaults

class CliOptions
{
	//
	// Public interface.
	//
public:
	//! Parses argv into a CliOptions instance; exits the process on invalid input.
	static CliOptions parse(const int argc, char** argv);
	//! Returns the configured payload size, in bytes.
	std::size_t payloadSize() const
	{
		return payloadSize_;
	}
	//! Returns the configured ring buffer size, in bytes.
	std::uint64_t ringBufferBytes() const
	{
		return ringBufferBytes_;
	}

	//
	// Private data members.
	//
private:
	//! Payload size in bytes, from the required positional argument.
	std::size_t payloadSize_{ defaults::payloadSize };
	//! Ring buffer size in bytes, from --buffer-size (default 64MiB).
	std::uint64_t ringBufferBytes_{ defaults::ringBufferBytes };
};

}	 // namespace producer
