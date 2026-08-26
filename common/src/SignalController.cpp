#include "SignalController.h"

#include <csignal>

namespace common
{

std::atomic<bool> SignalController::paused_{ false };
std::atomic<bool> SignalController::stopRequested_{ false };

void SignalController::install() const
{
	struct sigaction pauseAction
	{
	};
	pauseAction.sa_handler = &SignalController::handlePause;
	sigemptyset(&pauseAction.sa_mask);
	sigaction(SIGUSR1, &pauseAction, nullptr);

	struct sigaction resumeAction
	{
	};
	resumeAction.sa_handler = &SignalController::handleResume;
	sigemptyset(&resumeAction.sa_mask);
	sigaction(SIGUSR2, &resumeAction, nullptr);

	struct sigaction stopAction
	{
	};
	stopAction.sa_handler = &SignalController::handleStop;
	sigemptyset(&stopAction.sa_mask);
	sigaction(SIGINT, &stopAction, nullptr);
	sigaction(SIGTERM, &stopAction, nullptr);
}

bool SignalController::isPaused() const
{
	return paused_.load(std::memory_order_acquire);
}

bool SignalController::isStopRequested() const
{
	return stopRequested_.load(std::memory_order_acquire);
}

void SignalController::handlePause([[maybe_unused]] const int signalNumber)
{
	paused_.store(true, std::memory_order_release);
}

void SignalController::handleResume([[maybe_unused]] const int signalNumber)
{
	paused_.store(false, std::memory_order_release);
}

void SignalController::handleStop([[maybe_unused]] const int signalNumber)
{
	stopRequested_.store(true, std::memory_order_release);
}

}	 // namespace common
