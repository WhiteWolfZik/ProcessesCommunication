#pragma once

#include <cstdint>

namespace common
{

struct PacketHeader
{
	//
	// Public data members.
	//
public:
	//! Monotonically increasing packet number assigned by Producer.
	std::uint64_t sequenceNumber;
	//! Time the packet was generated, in nanoseconds.
	std::uint64_t timestampNs;
	//! Size of the payload that follows the header, in bytes.
	std::uint32_t payloadSize;
	//! CRC32 checksum over the header fields and payload.
	std::uint32_t checksum;
};

}	 // namespace common
