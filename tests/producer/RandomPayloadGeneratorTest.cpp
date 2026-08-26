#include "RandomPayloadGenerator.h"

#include <cstddef>

#include <gtest/gtest.h>

/*
 * Brief: generate(0) returns an empty buffer without crashing.
 * Given: A fresh RandomPayloadGenerator.
 * When:  generate(0) is called.
 * Then:  The returned vector is empty.
 */
TEST(RandomPayloadGeneratorTest, GenerateZeroSizeIsEmpty)
{
	producer::RandomPayloadGenerator generator;

	EXPECT_TRUE(generator.generate(0).empty());
}

/*
 * Brief: generate(1) returns exactly one byte.
 * Given: A fresh RandomPayloadGenerator.
 * When:  generate(1) is called.
 * Then:  The returned vector has size 1.
 */
TEST(RandomPayloadGeneratorTest, GenerateOneByte)
{
	producer::RandomPayloadGenerator generator;

	EXPECT_EQ(generator.generate(1).size(), std::size_t{ 1 });
}

/*
 * Brief: generate() returns a buffer of exactly the requested size for a
 *        large request.
 * Given: A fresh RandomPayloadGenerator.
 * When:  generate() is called with a 1 MiB size.
 * Then:  The returned vector's size matches exactly.
 */
TEST(RandomPayloadGeneratorTest, GenerateLargeSize)
{
	constexpr std::size_t largeSize{ 1024 * 1024 };
	producer::RandomPayloadGenerator generator;

	EXPECT_EQ(generator.generate(largeSize).size(), largeSize);
}

/*
 * Brief: Two consecutive calls on the same generator produce different content.
 * Given: A fresh RandomPayloadGenerator.
 * When:  generate(64) is called twice.
 * Then:  The two buffers differ (collision probability is astronomically
 *        small at this size, so this reliably catches a stuck/non-advancing
 *        RNG state).
 */
TEST(RandomPayloadGeneratorTest, ConsecutiveCallsDiffer)
{
	producer::RandomPayloadGenerator generator;

	const auto first{ generator.generate(64) };
	const auto second{ generator.generate(64) };

	EXPECT_NE(first, second);
}

/*
 * Brief: Two independently constructed generators produce different streams.
 * Given: Two separate RandomPayloadGenerator instances.
 * When:  generate(64) is called once on each.
 * Then:  The two buffers differ, confirming each instance is independently
 *        seeded from std::random_device rather than sharing a fixed seed.
 */
TEST(RandomPayloadGeneratorTest, SeparateInstancesProduceDifferentStreams)
{
	producer::RandomPayloadGenerator generatorA;
	producer::RandomPayloadGenerator generatorB;

	const auto fromA{ generatorA.generate(64) };
	const auto fromB{ generatorB.generate(64) };

	EXPECT_NE(fromA, fromB);
}
