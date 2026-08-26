#include "SignalController.h"

#include <csignal>

#include <gtest/gtest.h>

namespace
{

bool hasNonDefaultHandler(const int signalNumber)
{
	struct sigaction current
	{
	};
	::sigaction(signalNumber, nullptr, &current);
	return current.sa_handler != SIG_DFL && current.sa_handler != SIG_IGN;
}

}	 // namespace

/*
 * Brief: install() registers real handlers for all four signals.
 * Given: A freshly constructed SignalController.
 * When:  install() is called.
 * Then:  SIGUSR1, SIGUSR2, SIGINT and SIGTERM all have a non-default,
 *        non-ignored handler installed (checked via sigaction(), not by
 *        raising every signal).
 */
TEST(SignalControllerTest, InstallRegistersAllFourSignals)
{
	common::SignalController signals;
	signals.install();

	EXPECT_TRUE(hasNonDefaultHandler(SIGUSR1));
	EXPECT_TRUE(hasNonDefaultHandler(SIGUSR2));
	EXPECT_TRUE(hasNonDefaultHandler(SIGINT));
	EXPECT_TRUE(hasNonDefaultHandler(SIGTERM));
}

/*
 * Brief: SIGUSR1 sets the pause flag.
 * Given: install()ed handlers, explicitly driven to the resumed state first.
 * When:  SIGUSR1 is raised.
 * Then:  isPaused() reports true.
 */
TEST(SignalControllerTest, Sigusr1SetsPaused)
{
	common::SignalController signals;
	signals.install();
	::raise(SIGUSR2);
	ASSERT_FALSE(signals.isPaused());

	::raise(SIGUSR1);

	EXPECT_TRUE(signals.isPaused());
}

/*
 * Brief: SIGUSR2 clears the pause flag.
 * Given: install()ed handlers, explicitly driven to the paused state first.
 * When:  SIGUSR2 is raised.
 * Then:  isPaused() reports false.
 */
TEST(SignalControllerTest, Sigusr2ClearsPaused)
{
	common::SignalController signals;
	signals.install();
	::raise(SIGUSR1);
	ASSERT_TRUE(signals.isPaused());

	::raise(SIGUSR2);

	EXPECT_FALSE(signals.isPaused());
}

/*
 * Brief: togglePause() flips the pause flag in both directions.
 * Given: install()ed handlers, explicitly driven to the resumed state first.
 * When:  togglePause() is called twice in a row.
 * Then:  The flag goes false -> true -> false.
 */
TEST(SignalControllerTest, TogglePauseFlipsBothWays)
{
	common::SignalController signals;
	signals.install();
	::raise(SIGUSR2);
	ASSERT_FALSE(signals.isPaused());

	signals.togglePause();
	EXPECT_TRUE(signals.isPaused());

	signals.togglePause();
	EXPECT_FALSE(signals.isPaused());
}

/*
 * Brief: SIGINT sets the stop-requested flag.
 * Given: install()ed handlers. stopRequested_ has no reset, so this is the
 *        only test in the suite that touches it.
 * When:  SIGINT is raised.
 * Then:  isStopRequested() reports true.
 */
TEST(SignalControllerTest, SigintSetsStopRequested)
{
	common::SignalController signals;
	signals.install();

	::raise(SIGINT);

	EXPECT_TRUE(signals.isStopRequested());
}
