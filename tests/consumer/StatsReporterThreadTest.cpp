#include "StatsReporterThread.h"

#include <chrono>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "SignalController.h"
#include "StatsCollector.h"

namespace
{

class CoutRedirectGuard
{
public:
	CoutRedirectGuard()
		: originalBuf_{ std::cout.rdbuf(captured_.rdbuf()) }
	{
	}

	~CoutRedirectGuard()
	{
		std::cout.rdbuf(originalBuf_);
	}

	CoutRedirectGuard(const CoutRedirectGuard&) = delete;
	CoutRedirectGuard& operator=(const CoutRedirectGuard&) = delete;

	std::string captured() const
	{
		return captured_.str();
	}

private:
	std::ostringstream captured_;
	std::streambuf* originalBuf_;
};

}	 // namespace

/*
 * Brief: A running reporter prints one correctly-formatted line reflecting
 *        recorded stats after one reporting interval elapses.
 * Given: A StatsCollector with two recorded packets (one valid, one
 *        invalid) and a SignalController explicitly driven to the resumed
 *        state first (its pause flag is static/shared across tests).
 * When:  StatsReporterThread is started with a 1 second interval and left
 *        running for slightly over one interval.
 * Then:  Captured stdout contains a line with "total=2" and "invalid=1".
 */
TEST(StatsReporterThreadTest, PrintsOneLineAfterOneInterval)
{
	consumer::StatsCollector stats;
	common::SignalController signals;
	signals.install();
	::raise(SIGUSR2);
	ASSERT_FALSE(signals.isPaused());
	stats.record(1000, true);
	stats.record(500, false);

	std::string output;
	{
		CoutRedirectGuard guard;
		consumer::StatsReporterThread reporter{ stats, signals, std::chrono::seconds(1) };
		reporter.start();
		std::this_thread::sleep_for(std::chrono::milliseconds(1300));
		reporter.stop();
		output = guard.captured();
	}

	EXPECT_NE(output.find("total=2"), std::string::npos) << output;
	EXPECT_NE(output.find("invalid=1"), std::string::npos) << output;
}

/*
 * Brief: No report line is printed while SignalController reports paused.
 * Given: A SignalController explicitly driven to the paused state.
 * When:  StatsReporterThread is started with a 1 second interval and left
 *        running for slightly over one interval.
 * Then:  Captured stdout is empty.
 */
TEST(StatsReporterThreadTest, NoLinePrintedWhilePaused)
{
	consumer::StatsCollector stats;
	common::SignalController signals;
	signals.install();
	::raise(SIGUSR1);
	ASSERT_TRUE(signals.isPaused());
	stats.record(100, true);

	std::string output;
	{
		CoutRedirectGuard guard;
		consumer::StatsReporterThread reporter{ stats, signals, std::chrono::seconds(1) };
		reporter.start();
		std::this_thread::sleep_for(std::chrono::milliseconds(1300));
		reporter.stop();
		output = guard.captured();
	}

	EXPECT_TRUE(output.empty()) << output;

	::raise(SIGUSR2);
}
