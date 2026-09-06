#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <set>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "ChecksumCalculator.h"
#include "PacketFixtures.h"
#include "PacketValidator.h"
#include "RingBufferLayout.h"
#include "SharedRingBuffer.h"
#include "SharedRingBufferReader.h"
#include "SignalController.h"
#include "TestSignalController.h"

namespace
{

std::string makeSegmentName(const std::string& suffix)
{
	return "/pc_test_" + std::to_string(::getpid()) + "_" + suffix;
}

}	 // namespace

/*
 * Brief: A packet published on the writer side arrives byte-for-byte
 *        identical on the reader side.
 * Given: An open SharedRingBuffer and an attached SharedRingBufferReader on
 *        the same fresh segment.
 * When:  20 packets are published and consumed one at a time, in lockstep.
 * Then:  Every consumed packet's header and payload exactly match what was
 *        published for that sequence number.
 */
TEST(RingBufferRoundTripTest, BasicSequentialPublishAndConsume)
{
	const auto name{ makeSegmentName("Basic") };
	constexpr std::size_t payloadSize{ 64 };
	constexpr std::uint64_t ringBufferBytes{ 64 * 1024 };

	producer::SharedRingBuffer writer;
	writer.open(name, ringBufferBytes, payloadSize);
	consumer::SharedRingBufferReader reader;
	tests::TestSignalController::resetForTesting();
	common::SignalController signals;
	ASSERT_TRUE(reader.attach(name, signals));

	consumer::Packet received;
	constexpr std::uint64_t packetCount{ 20 };
	for (std::uint64_t sequenceNumber{ 0 }; sequenceNumber < packetCount; ++sequenceNumber)
	{
		const auto packet{ tests::makeValidPacket(sequenceNumber, payloadSize) };
		writer.publish(packet.header, std::span<const std::uint8_t>(packet.payload));

		ASSERT_TRUE(reader.tryConsume(received)) << "sequenceNumber=" << sequenceNumber;
		EXPECT_EQ(received.header.sequenceNumber, sequenceNumber);
		EXPECT_EQ(received.header.checksum, packet.header.checksum);
		EXPECT_EQ(received.payload, packet.payload);
	}
}

/*
 * Brief: Overwrite-oldest under a burst larger than capacity never
 *        duplicates or corrupts data — only the packets that are still
 *        physically present get delivered, each exactly once.
 * Given: A ring buffer sized for exactly 4 slots. Three packets are
 *        published and read normally first, so PacketValidator has an
 *        established lastSequenceNumber_ before the burst hits.
 * When:  20 more packets are published back-to-back with no reads in
 *        between (far more than capacity), then everything is drained.
 * Then:  Exactly capacity (4) more packets are delivered — the ones
 *        actually still resident — each with a correct checksum and a
 *        distinct sequenceNumber (no duplicate delivery), the first of
 *        them is flagged as a SequenceGap (the skip is now detectable
 *        because it isn't PacketValidator's very first packet), and no
 *        packet is ever flagged as a checksum or payload-size mismatch.
 */
TEST(RingBufferRoundTripTest, OverwriteOldestBurstNeverDuplicatesOrCorruptsSurvivors)
{
	const auto name{ makeSegmentName("OverwriteBurst") };
	constexpr std::size_t payloadSize{ 16 };
	constexpr std::uint64_t capacityTarget{ 4 };
	const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
	const std::uint64_t ringBufferBytes{ sizeof(common::RingBufferControlBlock)
										 + stride * capacityTarget };

	producer::SharedRingBuffer writer;
	writer.open(name, ringBufferBytes, payloadSize);
	consumer::SharedRingBufferReader reader;
	tests::TestSignalController::resetForTesting();
	common::SignalController signals;
	ASSERT_TRUE(reader.attach(name, signals));

	consumer::PacketValidator validator;
	consumer::Packet received;
	std::vector<std::uint64_t> deliveredSequenceNumbers;
	std::uint64_t gapCount{ 0 };

	auto drainOne{ [&]() -> bool {
		if (!reader.tryConsume(received))
		{
			return false;
		}

		const auto recomputed{ common::ChecksumCalculator::compute(
			received.header, std::as_bytes(std::span<const std::uint8_t>(received.payload))) };
		EXPECT_EQ(recomputed, received.header.checksum)
			<< "sequenceNumber=" << received.header.sequenceNumber;

		deliveredSequenceNumbers.push_back(received.header.sequenceNumber);

		const auto result{ validator.validate(
			received.header, std::span<const std::uint8_t>(received.payload)) };
		EXPECT_NE(result, consumer::ValidationResult::ChecksumMismatch);
		EXPECT_NE(result, consumer::ValidationResult::PayloadSizeMismatch);
		if (result == consumer::ValidationResult::SequenceGap)
		{
			++gapCount;
		}
		return true;
	} };

	constexpr std::uint64_t warmupCount{ 3 };
	for (std::uint64_t sequenceNumber{ 0 }; sequenceNumber < warmupCount; ++sequenceNumber)
	{
		const auto packet{ tests::makeValidPacket(sequenceNumber, payloadSize) };
		writer.publish(packet.header, std::span<const std::uint8_t>(packet.payload));
		ASSERT_TRUE(drainOne()) << "sequenceNumber=" << sequenceNumber;
	}

	constexpr std::uint64_t burstCount{ 20 };
	for (std::uint64_t sequenceNumber{ warmupCount }; sequenceNumber < warmupCount + burstCount;
		 ++sequenceNumber)
	{
		const auto packet{ tests::makeValidPacket(sequenceNumber, payloadSize) };
		writer.publish(packet.header, std::span<const std::uint8_t>(packet.payload));
	}

	const auto expectedTotal{ warmupCount + capacityTarget };
	for (std::uint64_t attempt{ 0 };
		 attempt < burstCount * 2 && deliveredSequenceNumbers.size() < expectedTotal;
		 ++attempt)
	{
		drainOne();
	}

	ASSERT_EQ(deliveredSequenceNumbers.size(), expectedTotal);

	std::set<std::uint64_t> distinct{ deliveredSequenceNumbers.begin(),
									  deliveredSequenceNumbers.end() };
	EXPECT_EQ(distinct.size(), deliveredSequenceNumbers.size()) << "duplicate delivery detected";

	EXPECT_EQ(gapCount, 1U);
}

/*
 * Brief: Concurrent writer and reader threads never produce a torn read.
 * Given: A ring buffer sized to hold every packet this test publishes
 *        (capacity == publishCount), so no overwrite can occur and the
 *        reader is guaranteed to eventually receive all of them regardless
 *        of scheduling.
 * When:  A writer thread publishes 5000 packets while a reader thread
 *        concurrently drains them via tryConsume() in a loop.
 * Then:  All 5000 packets are received and every one's checksum,
 *        recomputed independently on the reader side, matches the header
 *        — proving the seqlock never let a half-written slot be read.
 */
TEST(RingBufferRoundTripTest, ConcurrentWriterAndReaderNeverProduceTornReads)
{
	const auto name{ makeSegmentName("Concurrent") };
	constexpr std::size_t payloadSize{ 64 };
	constexpr std::uint64_t publishCount{ 5000 };
	const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
	const std::uint64_t ringBufferBytes{ sizeof(common::RingBufferControlBlock)
										 + stride * publishCount };

	producer::SharedRingBuffer writer;
	writer.open(name, ringBufferBytes, payloadSize);
	consumer::SharedRingBufferReader reader;
	tests::TestSignalController::resetForTesting();
	common::SignalController signals;
	ASSERT_TRUE(reader.attach(name, signals));

	std::thread producerThread([&writer]() {
		for (std::uint64_t sequenceNumber{ 0 }; sequenceNumber < publishCount; ++sequenceNumber)
		{
			const auto packet{ tests::makeValidPacket(sequenceNumber, payloadSize) };
			writer.publish(packet.header, std::span<const std::uint8_t>(packet.payload));
		}
	});

	consumer::Packet received;
	std::uint64_t receivedCount{ 0 };
	std::uint64_t checksumFailures{ 0 };
	while (receivedCount < publishCount)
	{
		if (!reader.tryConsume(received))
		{
			continue;
		}
		++receivedCount;

		const auto recomputed{ common::ChecksumCalculator::compute(
			received.header, std::as_bytes(std::span<const std::uint8_t>(received.payload))) };
		if (recomputed != received.header.checksum)
		{
			++checksumFailures;
		}
	}

	producerThread.join();

	EXPECT_EQ(checksumFailures, 0U);
	EXPECT_EQ(receivedCount, publishCount);
}

/*
 * Brief: attach() genuinely blocks and retries until the segment exists,
 *        rather than failing immediately.
 * Given: A SharedRingBufferReader attaching to a segment that does not
 *        exist yet, on a background thread.
 * When:  The main thread waits 250ms (well over one retry interval) before
 *        creating the segment via SharedRingBuffer::open().
 * Then:  attach() has not returned during that wait, and returns promptly
 *        once the segment is created.
 */
TEST(RingBufferRoundTripTest, ReaderAttachRetriesUntilProducerCreatesSegment)
{
	const auto name{ makeSegmentName("AttachRetry") };
	constexpr std::size_t payloadSize{ 32 };
	constexpr std::uint64_t ringBufferBytes{ 64 * 1024 };

	consumer::SharedRingBufferReader reader;
	tests::TestSignalController::resetForTesting();
	common::SignalController signals;
	std::atomic<bool> attached{ false };
	std::thread attachThread([&]() {
		attached.store(reader.attach(name, signals), std::memory_order_release);
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	EXPECT_FALSE(attached.load(std::memory_order_acquire));

	producer::SharedRingBuffer writer;
	writer.open(name, ringBufferBytes, payloadSize);

	attachThread.join();
	EXPECT_TRUE(attached.load(std::memory_order_acquire));
}

/*
 * Brief: attach() never even starts retrying once a stop has already been
 *        requested — this is what lets Ctrl-C interrupt a consumer that is
 *        still waiting for a producer to appear, instead of the retry loop
 *        ignoring the signal forever.
 * Given: A SignalController with a stop already requested (forced via
 *        raise(SIGTERM), which is safe regardless of whatever state earlier
 *        tests left the shared stop flag in, since it only ever moves
 *        false -> true) and a segment name that is never created.
 * When:  attach() is called against that segment.
 * Then:  It returns false immediately, without waiting through even one
 *        retry interval.
 */
TEST(RingBufferRoundTripTest, ReaderAttachAbortsWhenStopAlreadyRequested)
{
	const auto name{ makeSegmentName("AttachAbort") };

	common::SignalController signals;
	signals.install();
	::raise(SIGTERM);
	ASSERT_TRUE(signals.isStopRequested());

	consumer::SharedRingBufferReader reader;
	const auto start{ std::chrono::steady_clock::now() };
	const auto result{ reader.attach(name, signals) };
	const auto elapsed{ std::chrono::steady_clock::now() - start };

	EXPECT_FALSE(result);
	EXPECT_LT(elapsed, std::chrono::milliseconds(50));
}
