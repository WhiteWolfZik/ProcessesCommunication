#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "PacketHeader.h"

namespace common
{

class ChecksumCalculator
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	ChecksumCalculator() = delete;

	//
	// Public interface.
	//
public:
	//! Computes the CRC32 checksum over the header fields and payload.
	static std::uint32_t compute(
		const PacketHeader& header, const std::span<const std::byte> payload);
};

}	 // namespace common
