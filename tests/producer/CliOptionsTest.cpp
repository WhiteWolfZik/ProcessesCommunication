#include "producer/include/CliOptions.h"

#include <cstdlib>

#include <gtest/gtest.h>

#include "ArgvBuilder.h"

namespace
{

using tests::ArgvBuilder;

}	 // namespace

/*
 * Brief: A bare payloadSize argument parses with the default buffer size.
 * Given: argv = { program, "64" }.
 * When:  CliOptions::parse() is called.
 * Then:  payloadSize() is 64 and ringBufferBytes() is the default.
 */
TEST(ProducerCliOptionsTest, DefaultBufferSizeWhenOnlyPayloadSizeGiven)
{
	ArgvBuilder args{ { "producer", "64" } };

	const auto options{ producer::CliOptions::parse(args.argc(), args.argv()) };

	EXPECT_EQ(options.payloadSize(), 64U);
	EXPECT_EQ(options.ringBufferBytes(), producer::defaults::ringBufferBytes);
}

/*
 * Brief: --buffer-size overrides the default ring buffer size.
 * Given: argv = { program, "128", "--buffer-size", "1048576" }.
 * When:  CliOptions::parse() is called.
 * Then:  payloadSize() is 128 and ringBufferBytes() is 1048576.
 */
TEST(ProducerCliOptionsTest, BufferSizeOverrideIsApplied)
{
	ArgvBuilder args{ { "producer", "128", "--buffer-size", "1048576" } };

	const auto options{ producer::CliOptions::parse(args.argc(), args.argv()) };

	EXPECT_EQ(options.payloadSize(), 128U);
	EXPECT_EQ(options.ringBufferBytes(), 1048576ULL);
}

/*
 * Brief: Missing the required positional payloadSize argument exits.
 * Given: argv = { program } (no positional argument at all).
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE and prints a usage message.
 */
TEST(ProducerCliOptionsTest, MissingPayloadSizeExits)
{
	ArgvBuilder args{ { "producer" } };

	EXPECT_EXIT(
		producer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: A payloadSize of "0" is rejected.
 * Given: argv = { program, "0" }.
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ProducerCliOptionsTest, ZeroPayloadSizeExits)
{
	ArgvBuilder args{ { "producer", "0" } };

	EXPECT_EXIT(
		producer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: A non-numeric payloadSize is rejected.
 * Given: argv = { program, "notanumber" }.
 * When:  CliOptions::parse() is called.
 * Then:  strtoull() parses it as 0, which is rejected: the process exits
 *        with EXIT_FAILURE.
 */
TEST(ProducerCliOptionsTest, NonNumericPayloadSizeExits)
{
	ArgvBuilder args{ { "producer", "notanumber" } };

	EXPECT_EXIT(
		producer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: --buffer-size with no following value is rejected.
 * Given: argv = { program, "64", "--buffer-size" } (flag with nothing after it).
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ProducerCliOptionsTest, BufferSizeFlagWithoutValueExits)
{
	ArgvBuilder args{ { "producer", "64", "--buffer-size" } };

	EXPECT_EXIT(
		producer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: An unrecognized flag is rejected.
 * Given: argv = { program, "64", "--not-a-real-flag" }.
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ProducerCliOptionsTest, UnknownFlagExits)
{
	ArgvBuilder args{ { "producer", "64", "--not-a-real-flag" } };

	EXPECT_EXIT(
		producer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}
