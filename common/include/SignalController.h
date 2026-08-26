#pragma once

#include <atomic>

namespace common
{

class SignalController
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	SignalController() = default;

	//
	// Public interface.
	//
public:
	//! Registers the SIGUSR1/SIGUSR2/SIGINT/SIGTERM handlers with sigaction.
	void install() const;
	//! Returns whether a pause is currently in effect.
	bool isPaused() const;
	//! Returns whether a graceful shutdown has been requested.
	bool isStopRequested() const;
	//! Flips the pause flag; used by KeypressListener as a signal-free alternative to
	//! SIGUSR1/SIGUSR2.
	void togglePause() const;

	//
	// Private methods.
	//
private:
	//! SIGUSR1 handler: sets the pause flag.
	static void handlePause(const int signalNumber);
	//! SIGUSR2 handler: clears the pause flag.
	static void handleResume(const int signalNumber);
	//! SIGINT/SIGTERM handler: sets the stop-requested flag.
	static void handleStop(const int signalNumber);

	//
	// Private data members.
	//
private:
	//! Shared pause flag, toggled from signal handlers.
	static std::atomic<bool> paused_;
	//! Shared stop-requested flag, set from signal handlers.
	static std::atomic<bool> stopRequested_;
};

}	 // namespace common
