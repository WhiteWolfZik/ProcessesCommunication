#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "ChecksumCalculator.h"
#include "PacketHeader.h"

namespace tests
{

//! A header/payload pair with a correctly computed checksum, ready to hand
//! to PacketValidator::validate() or SharedRingBuffer::publish().
struct ValidPacket
{
	common::PacketHeader header;
	std::vector<std::uint8_t> payload;
};

//! Builds a valid packet: payload is a deterministic byte pattern derived
//! from sequenceNumber, and header.checksum is computed to match it.
inline ValidPacket makeValidPacket(
	const std::uint64_t sequenceNumber, const std::size_t payloadSize)
{
	ValidPacket packet;
	packet.payload.resize(payloadSize);
	for (std::size_t i{ 0 }; i < payloadSize; ++i)
	{
		packet.payload[i] = static_cast<std::uint8_t>((sequenceNumber + i) % 256);
	}

	packet.header.sequenceNumber = sequenceNumber;
	packet.header.timestampNs = sequenceNumber * 1000ULL;
	packet.header.payloadSize = static_cast<std::uint32_t>(payloadSize);
	packet.header.checksum = common::ChecksumCalculator::compute(
		packet.header, std::as_bytes(std::span<const std::uint8_t>(packet.payload)));

	return packet;
}

}	 // namespace tests
