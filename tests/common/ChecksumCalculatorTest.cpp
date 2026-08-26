#include "ChecksumCalculator.h"

#include <cstdint>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "PacketHeader.h"

namespace
{

common::PacketHeader makeHeader(
	const std::uint64_t sequenceNumber,
	const std::uint64_t timestampNs,
	const std::uint32_t payloadSize)
{
	common::PacketHeader header{};
	header.sequenceNumber = sequenceNumber;
	header.timestampNs = timestampNs;
	header.payloadSize = payloadSize;
	header.checksum = 0;
	return header;
}

}	 // namespace

/*
 * Brief: Checksum computation is deterministic.
 * Given: A fixed header and payload.
 * When:  compute() is called twice with identical inputs.
 * Then:  Both calls return the same checksum value.
 */
TEST(ChecksumCalculatorTest, SameInputProducesSameChecksum)
{
	const auto header{ makeHeader(1, 100, 4) };
	const std::vector<std::uint8_t> payload{ 1, 2, 3, 4 };

	const auto first{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto second{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_EQ(first, second);
}

/*
 * Brief: Checksum is stable and non-crashing for an empty payload.
 * Given: A header with payloadSize 0 and an empty payload span.
 * When:  compute() is called.
 * Then:  It returns a valid, reproducible checksum without crashing.
 */
TEST(ChecksumCalculatorTest, EmptyPayloadIsStable)
{
	const auto header{ makeHeader(1, 100, 0) };
	const std::vector<std::uint8_t> payload{};

	const auto first{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto second{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_EQ(first, second);
}

/*
 * Brief: Changing sequenceNumber changes the checksum.
 * Given: Two headers identical except for sequenceNumber, same payload.
 * When:  compute() is called for each.
 * Then:  The two checksums differ, proving sequenceNumber feeds the hash.
 */
TEST(ChecksumCalculatorTest, SequenceNumberAffectsChecksum)
{
	const std::vector<std::uint8_t> payload{ 9, 8, 7 };
	const auto headerA{ makeHeader(1, 100, 3) };
	const auto headerB{ makeHeader(2, 100, 3) };

	const auto checksumA{ common::ChecksumCalculator::compute(
		headerA, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto checksumB{ common::ChecksumCalculator::compute(
		headerB, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_NE(checksumA, checksumB);
}

/*
 * Brief: Changing timestampNs changes the checksum.
 * Given: Two headers identical except for timestampNs, same payload.
 * When:  compute() is called for each.
 * Then:  The two checksums differ, proving timestampNs feeds the hash.
 */
TEST(ChecksumCalculatorTest, TimestampAffectsChecksum)
{
	const std::vector<std::uint8_t> payload{ 9, 8, 7 };
	const auto headerA{ makeHeader(1, 100, 3) };
	const auto headerB{ makeHeader(1, 200, 3) };

	const auto checksumA{ common::ChecksumCalculator::compute(
		headerA, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto checksumB{ common::ChecksumCalculator::compute(
		headerB, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_NE(checksumA, checksumB);
}

/*
 * Brief: Changing payloadSize changes the checksum.
 * Given: Two headers identical except for payloadSize, same payload bytes hashed.
 * When:  compute() is called for each.
 * Then:  The two checksums differ, proving payloadSize feeds the hash.
 */
TEST(ChecksumCalculatorTest, PayloadSizeFieldAffectsChecksum)
{
	const std::vector<std::uint8_t> payload{ 9, 8, 7 };
	const auto headerA{ makeHeader(1, 100, 3) };
	const auto headerB{ makeHeader(1, 100, 30) };

	const auto checksumA{ common::ChecksumCalculator::compute(
		headerA, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto checksumB{ common::ChecksumCalculator::compute(
		headerB, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_NE(checksumA, checksumB);
}

/*
 * Brief: A single flipped payload byte changes the checksum, at any position.
 * Given: A baseline payload and three variants each differing by one byte
 *        (first, middle, last).
 * When:  compute() is called for the baseline and each variant.
 * Then:  Every variant's checksum differs from the baseline's.
 */
TEST(ChecksumCalculatorTest, SingleBytePayloadChangeAtAnyPositionChangesChecksum)
{
	const auto header{ makeHeader(1, 100, 5) };
	const std::vector<std::uint8_t> baseline{ 10, 20, 30, 40, 50 };
	const auto baselineChecksum{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(baseline))) };

	for (const std::size_t flipIndex : { std::size_t{ 0 }, std::size_t{ 2 }, std::size_t{ 4 } })
	{
		auto variant{ baseline };
		variant[flipIndex] = static_cast<std::uint8_t>(variant[flipIndex] + 1);

		const auto variantChecksum{ common::ChecksumCalculator::compute(
			header, std::as_bytes(std::span<const std::uint8_t>(variant))) };

		EXPECT_NE(baselineChecksum, variantChecksum) << "flipIndex=" << flipIndex;
	}
}

/*
 * Brief: A large payload is handled correctly and stably.
 * Given: An 8 KiB payload of a repeating pattern.
 * When:  compute() is called twice.
 * Then:  Both results match, confirming no overflow/crash on larger inputs.
 */
TEST(ChecksumCalculatorTest, LargePayloadIsStable)
{
	constexpr std::size_t largeSize{ 8192 };
	std::vector<std::uint8_t> payload(largeSize);
	for (std::size_t i{ 0 }; i < largeSize; ++i)
	{
		payload[i] = static_cast<std::uint8_t>(i % 256);
	}
	const auto header{ makeHeader(42, 12345, static_cast<std::uint32_t>(largeSize)) };

	const auto first{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };
	const auto second{ common::ChecksumCalculator::compute(
		header, std::as_bytes(std::span<const std::uint8_t>(payload))) };

	EXPECT_EQ(first, second);
}
