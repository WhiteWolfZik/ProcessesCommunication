#pragma once

#include <cstdint>
#include <span>

#include "PacketHeader.h"

namespace consumer
{

//! Outcome of validating one received packet.
enum class ValidationResult
{
	Valid,					//< All checks passed.
	PayloadSizeMismatch,	//< The received payload's length didn't match header.payloadSize.
	ChecksumMismatch,		//< Checksum did not match; the packet is corrupted.
	SequenceGap,			//< Checksum matched but a sequence number was skipped.
};

class PacketValidator
{
	//
	// Public interface.
	//
public:
	//! Verifies a packet's size, checksum and sequence continuity.
	ValidationResult validate(
		const common::PacketHeader& header, const std::span<const std::uint8_t> payload);

	//
	// Private data members.
	//
private:
	//! Sequence number of the last packet seen.
	std::uint64_t lastSequenceNumber_{ 0 };
	//! Whether a packet has been seen yet, to skip the gap check on the first one.
	bool hasSeenPacket_{ false };
};

}	 // namespace consumer
