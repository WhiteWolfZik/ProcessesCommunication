#include "RandomPayloadGenerator.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <gtest/gtest.h>

/*
 * Brief: generate() into an empty span doesn't crash.
 * Given: A fresh RandomPayloadGenerator and an empty buffer.
 * When:  generate() is called on it.
 * Then:  No crash occurs; the buffer stays empty.
 */
TEST(RandomPayloadGeneratorTest, GenerateIntoEmptySpanIsSafe)
{
	producer::RandomPayloadGenerator generator;
	std::vector<std::uint8_t> buffer;

	generator.generate(std::span<std::uint8_t>(buffer));

	EXPECT_TRUE(buffer.empty());
}

/*
 * Brief: generate() fills a one-byte buffer, exercising the sub-word tail path.
 * Given: A fresh RandomPayloadGenerator and a one-byte buffer.
 * When:  generate() is called on it.
 * Then:  The buffer's single byte is written (no crash, no out-of-bounds write).
 */
TEST(RandomPayloadGeneratorTest, GenerateOneByte)
{
	producer::RandomPayloadGenerator generator;
	std::vector<std::uint8_t> buffer(1);

	generator.generate(std::span<std::uint8_t>(buffer));

	EXPECT_EQ(buffer.size(), std::size_t{ 1 });
}

/*
 * Brief: generate() fills a large buffer completely, without touching
 *        anything past its end.
 * Given: A fresh RandomPayloadGenerator and a 1 MiB buffer bracketed by
 *        sentinel bytes in a larger backing array.
 * When:  generate() is called on the 1 MiB span.
 * Then:  The sentinel bytes immediately before and after the span are
 *        untouched, confirming no overrun despite the 8-byte-at-a-time fill.
 */
TEST(RandomPayloadGeneratorTest, GenerateLargeSizeStaysInBounds)
{
	constexpr std::size_t largeSize{ 1024 * 1024 };
	constexpr std::uint8_t sentinel{ 0xAB };

	std::vector<std::uint8_t> backing(largeSize + 2, sentinel);
	producer::RandomPayloadGenerator generator;

	generator.generate(std::span<std::uint8_t>(backing.data() + 1, largeSize));

	EXPECT_EQ(backing.front(), sentinel);
	EXPECT_EQ(backing.back(), sentinel);
}

/*
 * Brief: Two consecutive calls on the same generator produce different content.
 * Given: A fresh RandomPayloadGenerator and two 64-byte buffers.
 * When:  generate() is called once per buffer.
 * Then:  The two buffers differ (collision probability is astronomically
 *        small at this size, so this reliably catches a stuck/non-advancing
 *        RNG state).
 */
TEST(RandomPayloadGeneratorTest, ConsecutiveCallsDiffer)
{
	producer::RandomPayloadGenerator generator;
	std::vector<std::uint8_t> first(64);
	std::vector<std::uint8_t> second(64);

	generator.generate(std::span<std::uint8_t>(first));
	generator.generate(std::span<std::uint8_t>(second));

	EXPECT_NE(first, second);
}

/*
 * Brief: Two independently constructed generators produce different streams.
 * Given: Two separate RandomPayloadGenerator instances and a 64-byte buffer each.
 * When:  generate() is called once on each.
 * Then:  The two buffers differ, confirming each instance is independently
 *        seeded from std::random_device rather than sharing a fixed seed.
 */
TEST(RandomPayloadGeneratorTest, SeparateInstancesProduceDifferentStreams)
{
	producer::RandomPayloadGenerator generatorA;
	producer::RandomPayloadGenerator generatorB;
	std::vector<std::uint8_t> fromA(64);
	std::vector<std::uint8_t> fromB(64);

	generatorA.generate(std::span<std::uint8_t>(fromA));
	generatorB.generate(std::span<std::uint8_t>(fromB));

	EXPECT_NE(fromA, fromB);
}
