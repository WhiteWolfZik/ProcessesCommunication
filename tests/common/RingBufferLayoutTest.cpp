#include "RingBufferLayout.h"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

/*
 * Brief: alignUp leaves an already-aligned value unchanged.
 * Given: A value that is already a multiple of the alignment.
 * When:  alignUp() is called.
 * Then:  It returns the value unchanged.
 */
TEST(RingBufferLayoutTest, AlignUpAlreadyAlignedIsUnchanged)
{
	EXPECT_EQ(common::ringBufferLayout::alignUp(16, 8), 16U);
}

/*
 * Brief: alignUp rounds an unaligned value up to the next multiple.
 * Given: A value that is not a multiple of the alignment.
 * When:  alignUp() is called.
 * Then:  It returns the next higher multiple of the alignment.
 */
TEST(RingBufferLayoutTest, AlignUpRoundsUpUnalignedValue)
{
	EXPECT_EQ(common::ringBufferLayout::alignUp(17, 8), 24U);
	EXPECT_EQ(common::ringBufferLayout::alignUp(1, 8), 8U);
}

/*
 * Brief: alignUp of zero is zero.
 * Given: A value of zero.
 * When:  alignUp() is called with any alignment.
 * Then:  It returns zero.
 */
TEST(RingBufferLayoutTest, AlignUpOfZeroIsZero)
{
	EXPECT_EQ(common::ringBufferLayout::alignUp(0, 8), 0U);
}

/*
 * Brief: alignUp with alignment 1 is the identity function.
 * Given: An arbitrary value and alignment of 1.
 * When:  alignUp() is called.
 * Then:  It returns the value unchanged.
 */
TEST(RingBufferLayoutTest, AlignUpWithAlignmentOneIsIdentity)
{
	EXPECT_EQ(common::ringBufferLayout::alignUp(123, 1), 123U);
}

/*
 * Brief: slotStride is always a multiple of the layout alignment.
 * Given: Several different payload sizes, including zero.
 * When:  slotStride() is called for each.
 * Then:  Every result is an exact multiple of alignmentBytes and large
 *        enough to hold the slot header, packet header, and payload.
 */
TEST(RingBufferLayoutTest, SlotStrideIsAlignedAndLargeEnough)
{
	for (const std::size_t payloadSize :
		 { std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 64 }, std::size_t{ 4096 } })
	{
		const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
		EXPECT_EQ(stride % common::ringBufferLayout::alignmentBytes, 0U)
			<< "payloadSize=" << payloadSize;
		EXPECT_GE(stride, sizeof(common::SlotHeader) + sizeof(common::PacketHeader) + payloadSize)
			<< "payloadSize=" << payloadSize;
	}
}

/*
 * Brief: slotOffset places slot 0 at the start of the slot array.
 * Given: Index 0 and an arbitrary stride.
 * When:  slotOffset() is called.
 * Then:  It returns 0.
 */
TEST(RingBufferLayoutTest, SlotOffsetOfFirstSlotIsZero)
{
	EXPECT_EQ(common::ringBufferLayout::slotOffset(0, 128), 0U);
}

/*
 * Brief: slotOffset scales linearly with the slot index.
 * Given: A fixed stride and several increasing indices.
 * When:  slotOffset() is called for each index.
 * Then:  Each offset equals index * stride exactly.
 */
TEST(RingBufferLayoutTest, SlotOffsetScalesWithIndex)
{
	constexpr std::size_t stride{ 96 };
	EXPECT_EQ(common::ringBufferLayout::slotOffset(1, stride), stride);
	EXPECT_EQ(common::ringBufferLayout::slotOffset(5, stride), 5 * stride);
	EXPECT_EQ(common::ringBufferLayout::slotOffset(1000, stride), 1000 * stride);
}

/*
 * Brief: capacityFor returns 0 when the buffer can't even hold the control block.
 * Given: A ringBufferBytes value smaller than sizeof(RingBufferControlBlock).
 * When:  capacityFor() is called.
 * Then:  It returns 0.
 */
TEST(RingBufferLayoutTest, CapacityForTooSmallBufferIsZero)
{
	EXPECT_EQ(
		common::ringBufferLayout::capacityFor(sizeof(common::RingBufferControlBlock) - 1, 64), 0U);
	EXPECT_EQ(common::ringBufferLayout::capacityFor(0, 64), 0U);
}

/*
 * Brief: capacityFor returns exactly 1 when the buffer fits precisely one slot.
 * Given: A ringBufferBytes value equal to the control block plus exactly one stride.
 * When:  capacityFor() is called.
 * Then:  It returns 1.
 */
TEST(RingBufferLayoutTest, CapacityForExactlyOneSlot)
{
	constexpr std::size_t payloadSize{ 64 };
	const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
	const auto ringBufferBytes{ sizeof(common::RingBufferControlBlock) + stride };

	EXPECT_EQ(common::ringBufferLayout::capacityFor(ringBufferBytes, payloadSize), 1U);
}

/*
 * Brief: capacityFor matches manual division for a realistic multi-slot buffer.
 * Given: A ringBufferBytes value sized for several slots plus a partial remainder.
 * When:  capacityFor() is called.
 * Then:  It returns exactly (ringBufferBytes - controlBlockSize) / stride, truncated.
 */
TEST(RingBufferLayoutTest, CapacityForMultiSlotBufferMatchesManualMath)
{
	constexpr std::size_t payloadSize{ 128 };
	const auto stride{ common::ringBufferLayout::slotStride(payloadSize) };
	const std::uint64_t ringBufferBytes{ sizeof(common::RingBufferControlBlock) + (stride * 10)
										 + (stride / 2) };

	EXPECT_EQ(common::ringBufferLayout::capacityFor(ringBufferBytes, payloadSize), 10U);
}
