#include "PacketValidator.h"

#include <span>

#include <gtest/gtest.h>

#include "PacketFixtures.h"

using tests::makeValidPacket;

/*
 * Brief: A correctly formed first packet validates as Valid.
 * Given: A fresh PacketValidator and a packet with a matching checksum.
 * When:  validate() is called on it.
 * Then:  The result is ValidationResult::Valid.
 */
TEST(PacketValidatorTest, ValidFirstPacketIsValid)
{
	consumer::PacketValidator validator;
	const auto packet{ makeValidPacket(1, 32) };

	const auto result{ validator.validate(
		packet.header, std::span<const std::uint8_t>(packet.payload)) };

	EXPECT_EQ(result, consumer::ValidationResult::Valid);
}

/*
 * Brief: A payload shorter than header.payloadSize is caught before checksum
 *        is even considered.
 * Given: A valid packet whose payload span is truncated by one byte.
 * When:  validate() is called.
 * Then:  The result is ValidationResult::PayloadSizeMismatch.
 */
TEST(PacketValidatorTest, TruncatedPayloadIsSizeMismatch)
{
	consumer::PacketValidator validator;
	auto packet{ makeValidPacket(1, 32) };
	const std::span<const std::uint8_t> truncated{ packet.payload.data(),
												   packet.payload.size() - 1 };

	const auto result{ validator.validate(packet.header, truncated) };

	EXPECT_EQ(result, consumer::ValidationResult::PayloadSizeMismatch);
}

/*
 * Brief: A single corrupted payload byte is caught as a checksum mismatch.
 * Given: A valid packet with one payload byte flipped after the checksum
 *        was computed.
 * When:  validate() is called.
 * Then:  The result is ValidationResult::ChecksumMismatch.
 */
TEST(PacketValidatorTest, CorruptedPayloadByteIsChecksumMismatch)
{
	consumer::PacketValidator validator;
	auto packet{ makeValidPacket(1, 32) };
	packet.payload[10] = static_cast<std::uint8_t>(packet.payload[10] + 1);

	const auto result{ validator.validate(
		packet.header, std::span<const std::uint8_t>(packet.payload)) };

	EXPECT_EQ(result, consumer::ValidationResult::ChecksumMismatch);
}

/*
 * Brief: A corrupted header field is caught as a checksum mismatch.
 * Given: A valid packet whose sequenceNumber is changed after the checksum
 *        was computed for the original value.
 * When:  validate() is called.
 * Then:  The result is ValidationResult::ChecksumMismatch.
 */
TEST(PacketValidatorTest, CorruptedHeaderFieldIsChecksumMismatch)
{
	consumer::PacketValidator validator;
	auto packet{ makeValidPacket(1, 32) };
	packet.header.sequenceNumber = 999;

	const auto result{ validator.validate(
		packet.header, std::span<const std::uint8_t>(packet.payload)) };

	EXPECT_EQ(result, consumer::ValidationResult::ChecksumMismatch);
}

/*
 * Brief: The very first packet seen is never flagged as a sequence gap, no
 *        matter what its sequence number is.
 * Given: A fresh PacketValidator (hasSeenPacket_ is false) and a valid
 *        packet with a large, non-zero sequence number.
 * When:  validate() is called.
 * Then:  The result is Valid, not SequenceGap.
 */
TEST(PacketValidatorTest, FirstPacketNeverFlaggedAsGapRegardlessOfSequenceNumber)
{
	consumer::PacketValidator validator;
	const auto packet{ makeValidPacket(500, 16) };

	const auto result{ validator.validate(
		packet.header, std::span<const std::uint8_t>(packet.payload)) };

	EXPECT_EQ(result, consumer::ValidationResult::Valid);
}

/*
 * Brief: A skipped sequence number is flagged as a gap on the next call.
 * Given: A PacketValidator that has already seen sequence number 1.
 * When:  validate() is called with sequence number 5 (skipping 2, 3, 4).
 * Then:  The result is ValidationResult::SequenceGap.
 */
TEST(PacketValidatorTest, SkippedSequenceNumberIsGap)
{
	consumer::PacketValidator validator;
	const auto first{ makeValidPacket(1, 16) };
	validator.validate(first.header, std::span<const std::uint8_t>(first.payload));

	const auto second{ makeValidPacket(5, 16) };
	const auto result{ validator.validate(
		second.header, std::span<const std::uint8_t>(second.payload)) };

	EXPECT_EQ(result, consumer::ValidationResult::SequenceGap);
}

/*
 * Brief: Consecutive sequence numbers across several calls all stay Valid.
 * Given: A fresh PacketValidator.
 * When:  validate() is called for sequence numbers 1 through 5, in order.
 * Then:  Every call returns Valid, with no gap flagged.
 */
TEST(PacketValidatorTest, ConsecutiveSequenceNumbersStayValid)
{
	consumer::PacketValidator validator;

	for (std::uint64_t sequenceNumber{ 1 }; sequenceNumber <= 5; ++sequenceNumber)
	{
		const auto packet{ makeValidPacket(sequenceNumber, 8) };
		const auto result{ validator.validate(
			packet.header, std::span<const std::uint8_t>(packet.payload)) };
		EXPECT_EQ(result, consumer::ValidationResult::Valid) << "sequenceNumber=" << sequenceNumber;
	}
}
