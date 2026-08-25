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
	//! Registers the SIGUSR1/SIGUSR2 handlers with sigaction.
	void install() const;
	//! Returns whether a pause is currently in effect.
	bool isPaused() const;

	//
	// Private methods.
	//
private:
	//! SIGUSR1 handler: sets the pause flag.
	static void handlePause(const int signalNumber);
	//! SIGUSR2 handler: clears the pause flag.
	static void handleResume(const int signalNumber);

	//
	// Private data members.
	//
private:
	//! Shared pause flag, toggled from signal handlers.
	static std::atomic<bool> paused_;
};

}	 // namespace common
