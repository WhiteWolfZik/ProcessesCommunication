#include "PacketValidator.h"

#include <span>

#include "ChecksumCalculator.h"

namespace consumer
{

ValidationResult PacketValidator::validate(
	const common::PacketHeader& header, const std::span<const std::uint8_t> payload)
{
	if (payload.size() != header.payloadSize)
	{
		return ValidationResult::PayloadSizeMismatch;
	}

	const auto expectedChecksum{ common::ChecksumCalculator::compute(
		header, std::as_bytes(payload)) };

	if (expectedChecksum != header.checksum)
	{
		return ValidationResult::ChecksumMismatch;
	}

	ValidationResult result{ ValidationResult::Valid };
	if (hasSeenPacket_ && header.sequenceNumber != lastSequenceNumber_ + 1)
	{
		result = ValidationResult::SequenceGap;
	}

	lastSequenceNumber_ = header.sequenceNumber;
	hasSeenPacket_ = true;
	return result;
}

}	 // namespace consumer
