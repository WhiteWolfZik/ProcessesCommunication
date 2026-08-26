#include "KeypressListener.h"

#include <pty.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

#include "SignalController.h"

namespace
{

constexpr auto listenerSettleTime{ std::chrono::milliseconds(250) };

class StdinRedirectGuard
{
public:
	explicit StdinRedirectGuard(const int replacementFd)
		: savedFd_{ ::dup(STDIN_FILENO) }
	{
		::dup2(replacementFd, STDIN_FILENO);
	}

	~StdinRedirectGuard()
	{
		::dup2(savedFd_, STDIN_FILENO);
		::close(savedFd_);
	}

	StdinRedirectGuard(const StdinRedirectGuard&) = delete;
	StdinRedirectGuard& operator=(const StdinRedirectGuard&) = delete;

private:
	int savedFd_;
};

}	 // namespace

/*
 * Brief: A real keypress on a pseudo-terminal toggles the shared pause flag,
 *        and a second keypress toggles it back.
 * Given: STDIN_FILENO redirected to the slave side of a real pty, a
 *        KeypressListener started against it, pause explicitly driven to
 *        the resumed state first (SignalController's flag is static/shared
 *        across the whole test binary).
 * When:  A single byte is written to the pty's master side, twice in a row,
 *        with enough settle time for one poll cycle each.
 * Then:  isPaused() reports true after the first byte and false after the
 *        second.
 */
TEST(KeypressListenerTest, KeypressOnRealPtyTogglesPause)
{
	int masterFd{ -1 };
	int slaveFd{ -1 };
	ASSERT_EQ(::openpty(&masterFd, &slaveFd, nullptr, nullptr, nullptr), 0);

	{
		StdinRedirectGuard stdinGuard{ slaveFd };
		::close(slaveFd);

		common::SignalController signals;
		signals.install();
		common::KeypressListener listener{ signals };
		::raise(SIGUSR2);
		ASSERT_FALSE(signals.isPaused());

		listener.start();

		const std::uint8_t key{ 'x' };
		ASSERT_EQ(::write(masterFd, &key, sizeof(key)), 1);
		std::this_thread::sleep_for(listenerSettleTime);
		EXPECT_TRUE(signals.isPaused());

		ASSERT_EQ(::write(masterFd, &key, sizeof(key)), 1);
		std::this_thread::sleep_for(listenerSettleTime);
		EXPECT_FALSE(signals.isPaused());

		listener.stop();
	}

	::close(masterFd);
}

/*
 * Brief: start() on a non-terminal stdin is a safe no-op.
 * Given: STDIN_FILENO redirected to the read end of a plain pipe (not a tty,
 *        so tcgetattr() fails).
 * When:  start() is called, followed immediately by stop().
 * Then:  Neither call crashes or hangs (no listener thread was ever spawned).
 */
TEST(KeypressListenerTest, StartOnNonTtyStdinIsSafeNoOp)
{
	std::array<int, 2> pipeFds{ -1, -1 };
	ASSERT_EQ(::pipe(pipeFds.data()), 0);

	{
		StdinRedirectGuard stdinGuard{ pipeFds[0] };

		common::SignalController signals;
		common::KeypressListener listener{ signals };
		listener.start();
		listener.stop();
	}

	::close(pipeFds[0]);
	::close(pipeFds[1]);
	SUCCEED();
}

/*
 * Brief: stop() without a preceding start() is a safe no-op, and is
 *        idempotent when called twice.
 * Given: A freshly constructed KeypressListener that was never started.
 * When:  stop() is called twice in a row.
 * Then:  Neither call crashes or hangs.
 */
TEST(KeypressListenerTest, StopWithoutStartIsSafeAndIdempotent)
{
	common::SignalController signals;
	common::KeypressListener listener{ signals };

	listener.stop();
	listener.stop();

	SUCCEED();
}
