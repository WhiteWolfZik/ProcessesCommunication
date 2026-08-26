#include "StatsCollector.h"

#include <cstdint>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

/*
 * Brief: A single valid record is reflected exactly in the snapshot.
 * Given: A fresh StatsCollector.
 * When:  record(100, true) is called once.
 * Then:  totalPackets=1, totalBytes=100, invalidPackets=0.
 */
TEST(StatsCollectorTest, SingleValidRecord)
{
	consumer::StatsCollector stats;

	stats.record(100, true);

	const auto snapshot{ stats.snapshot() };
	EXPECT_EQ(snapshot.totalPackets, 1U);
	EXPECT_EQ(snapshot.totalBytes, 100U);
	EXPECT_EQ(snapshot.invalidPackets, 0U);
}

/*
 * Brief: An invalid record increments invalidPackets alongside the totals.
 * Given: A fresh StatsCollector.
 * When:  record(50, false) is called once.
 * Then:  totalPackets=1, totalBytes=50, invalidPackets=1.
 */
TEST(StatsCollectorTest, SingleInvalidRecord)
{
	consumer::StatsCollector stats;

	stats.record(50, false);

	const auto snapshot{ stats.snapshot() };
	EXPECT_EQ(snapshot.totalPackets, 1U);
	EXPECT_EQ(snapshot.totalBytes, 50U);
	EXPECT_EQ(snapshot.invalidPackets, 1U);
}

/*
 * Brief: A mix of valid and invalid records accumulates correctly.
 * Given: A fresh StatsCollector.
 * When:  Three valid records of 10 bytes and two invalid records of 20
 *        bytes are recorded, in sequence.
 * Then:  totalPackets=5, totalBytes=(3*10 + 2*20)=70, invalidPackets=2.
 */
TEST(StatsCollectorTest, MixedValidAndInvalidRecordsAccumulate)
{
	consumer::StatsCollector stats;

	for (std::int32_t i{ 0 }; i < 3; ++i)
	{
		stats.record(10, true);
	}
	for (std::int32_t i{ 0 }; i < 2; ++i)
	{
		stats.record(20, false);
	}

	const auto snapshot{ stats.snapshot() };
	EXPECT_EQ(snapshot.totalPackets, 5U);
	EXPECT_EQ(snapshot.totalBytes, 70U);
	EXPECT_EQ(snapshot.invalidPackets, 2U);
}

/*
 * Brief: Concurrent record() calls from many threads produce exact totals,
 *        proving the atomics hold up under real contention.
 * Given: A fresh StatsCollector and 8 threads, each recording 10000 packets
 *        of 4 bytes; every 3rd record on each thread is marked invalid.
 * When:  All threads run concurrently and are joined.
 * Then:  snapshot() reports totalPackets=80000, totalBytes=320000, and
 *        invalidPackets equal to the exact count of every-3rd records
 *        across all threads.
 */
TEST(StatsCollectorTest, ConcurrentRecordsProduceExactTotals)
{
	constexpr std::int32_t threadCount{ 8 };
	constexpr std::int32_t recordsPerThread{ 10000 };
	constexpr std::uint64_t bytesPerRecord{ 4 };

	consumer::StatsCollector stats;
	std::vector<std::thread> threads;
	for (std::int32_t t{ 0 }; t < threadCount; ++t)
	{
		threads.emplace_back([&stats]() {
			for (std::int32_t i{ 0 }; i < recordsPerThread; ++i)
			{
				const bool valid{ (i % 3) != 0 };
				stats.record(bytesPerRecord, valid);
			}
		});
	}
	for (auto& thread : threads)
	{
		thread.join();
	}

	std::uint64_t expectedInvalidPerThread{ 0 };
	for (std::int32_t i{ 0 }; i < recordsPerThread; ++i)
	{
		if (i % 3 == 0)
		{
			++expectedInvalidPerThread;
		}
	}

	const auto snapshot{ stats.snapshot() };
	EXPECT_EQ(snapshot.totalPackets, static_cast<std::uint64_t>(threadCount) * recordsPerThread);
	EXPECT_EQ(
		snapshot.totalBytes,
		static_cast<std::uint64_t>(threadCount) * recordsPerThread * bytesPerRecord);
	EXPECT_EQ(
		snapshot.invalidPackets,
		static_cast<std::uint64_t>(threadCount) * expectedInvalidPerThread);
}
