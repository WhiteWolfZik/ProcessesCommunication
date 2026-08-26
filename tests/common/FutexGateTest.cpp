#include "FutexGate.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

/*
 * Brief: wait() returns promptly when the word already differs from expected.
 * Given: A futex word whose current value is not the expected value.
 * When:  wait(expected) is called with no timeout.
 * Then:  It returns quickly (no real blocking), well under a small bound.
 */
TEST(FutexGateTest, WaitReturnsImmediatelyOnMismatch)
{
	std::atomic<std::uint32_t> word{ 5 };
	common::FutexGate gate{ word };

	const auto start{ std::chrono::steady_clock::now() };
	gate.wait(0);
	const auto elapsed{ std::chrono::steady_clock::now() - start };

	EXPECT_LT(elapsed, std::chrono::milliseconds(50));
}

/*
 * Brief: wait() with a matching word and no waker times out close to the
 *        requested duration instead of hanging forever.
 * Given: A futex word equal to the expected value, with nothing to wake it.
 * When:  wait(expected, timeout) is called with a 150ms timeout.
 * Then:  It returns after roughly 150ms, bounded well below a hang.
 */
TEST(FutexGateTest, WaitTimesOutCloseToRequestedDuration)
{
	std::atomic<std::uint32_t> word{ 0 };
	common::FutexGate gate{ word };

	const auto start{ std::chrono::steady_clock::now() };
	gate.wait(0, std::chrono::milliseconds(150));
	const auto elapsed{ std::chrono::steady_clock::now() - start };

	EXPECT_GE(elapsed, std::chrono::milliseconds(100));
	EXPECT_LT(elapsed, std::chrono::milliseconds(1000));
}

/*
 * Brief: wake() actually unblocks a thread genuinely parked in wait().
 * Given: A background thread blocked in wait() with a generous 2s safety
 *        timeout (so a broken wake() fails the assertion instead of hanging
 *        the test suite).
 * When:  The main thread changes the word and calls wake() after 100ms.
 * Then:  The background thread returns well before its 2s safety timeout,
 *        proving it was woken rather than merely timing out.
 */
TEST(FutexGateTest, WakeUnblocksRealWaiter)
{
	std::atomic<std::uint32_t> word{ 0 };
	common::FutexGate gate{ word };

	const auto start{ std::chrono::steady_clock::now() };
	std::thread waiter([&]() {
		gate.wait(0, std::chrono::milliseconds(2000));
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	word.store(1, std::memory_order_relaxed);
	gate.wake();

	waiter.join();
	const auto elapsed{ std::chrono::steady_clock::now() - start };

	EXPECT_LT(elapsed, std::chrono::milliseconds(1000));
}

/*
 * Brief: wake() with no waiters is a safe no-op.
 * Given: A futex word that nobody is waiting on.
 * When:  wake() is called.
 * Then:  It returns immediately without crashing or blocking.
 */
TEST(FutexGateTest, WakeWithNoWaitersIsSafeNoOp)
{
	std::atomic<std::uint32_t> word{ 0 };
	common::FutexGate gate{ word };

	const auto start{ std::chrono::steady_clock::now() };
	gate.wake();
	const auto elapsed{ std::chrono::steady_clock::now() - start };

	EXPECT_LT(elapsed, std::chrono::milliseconds(50));
}
