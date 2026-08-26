#include "consumer/include/CliOptions.h"

#include <chrono>
#include <cstdlib>

#include <gtest/gtest.h>

#include "ArgvBuilder.h"

namespace
{

using tests::ArgvBuilder;

}	 // namespace

/*
 * Brief: No arguments at all parses with the default reporting interval.
 * Given: argv = { program } (no flags).
 * When:  CliOptions::parse() is called.
 * Then:  reportInterval() is the default (1 second).
 */
TEST(ConsumerCliOptionsTest, DefaultIntervalWhenNoArgsGiven)
{
	ArgvBuilder args{ { "consumer" } };

	const auto options{ consumer::CliOptions::parse(args.argc(), args.argv()) };

	EXPECT_EQ(options.reportInterval(), consumer::defaults::reportInterval);
}

/*
 * Brief: --interval overrides the default reporting interval.
 * Given: argv = { program, "--interval", "5" }.
 * When:  CliOptions::parse() is called.
 * Then:  reportInterval() is 5 seconds.
 */
TEST(ConsumerCliOptionsTest, IntervalOverrideIsApplied)
{
	ArgvBuilder args{ { "consumer", "--interval", "5" } };

	const auto options{ consumer::CliOptions::parse(args.argc(), args.argv()) };

	EXPECT_EQ(options.reportInterval(), std::chrono::seconds(5));
}

/*
 * Brief: --interval 0 is rejected.
 * Given: argv = { program, "--interval", "0" }.
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ConsumerCliOptionsTest, ZeroIntervalExits)
{
	ArgvBuilder args{ { "consumer", "--interval", "0" } };

	EXPECT_EXIT(
		consumer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: --interval with no following value is rejected.
 * Given: argv = { program, "--interval" } (flag with nothing after it).
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ConsumerCliOptionsTest, IntervalFlagWithoutValueExits)
{
	ArgvBuilder args{ { "consumer", "--interval" } };

	EXPECT_EXIT(
		consumer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}

/*
 * Brief: An unrecognized flag is rejected.
 * Given: argv = { program, "--not-a-real-flag" }.
 * When:  CliOptions::parse() is called.
 * Then:  The process exits with EXIT_FAILURE.
 */
TEST(ConsumerCliOptionsTest, UnknownFlagExits)
{
	ArgvBuilder args{ { "consumer", "--not-a-real-flag" } };

	EXPECT_EXIT(
		consumer::CliOptions::parse(args.argc(), args.argv()),
		::testing::ExitedWithCode(EXIT_FAILURE),
		"Usage:");
}
