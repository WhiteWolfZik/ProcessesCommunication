#pragma once

#include <atomic>
#include <chrono>
#include <termios.h>
#include <thread>

namespace common
{

class SignalController;

namespace keypress
{

constexpr std::chrono::milliseconds pollTimeout{ 100 };

}	 // namespace keypress

//! Watches stdin in a background thread and toggles SignalController's pause
//! flag on any keypress, as an alternative to SIGUSR1/SIGUSR2. If stdin is
//! not a terminal, start() is a no-op: keypress pause simply isn't available,
//! and only the signal path still works.
class KeypressListener
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	explicit KeypressListener(const SignalController& signals);
	//! Destructor.
	~KeypressListener();
	//! Copy constructor.
	KeypressListener(const KeypressListener&) = delete;
	//! Copy assignment operator.
	KeypressListener& operator=(const KeypressListener&) = delete;
	//! Move constructor.
	KeypressListener(KeypressListener&&) = delete;
	//! Move assignment operator.
	KeypressListener& operator=(KeypressListener&&) = delete;

	//
	// Public interface.
	//
public:
	//! Puts the terminal into raw mode and starts the background listener thread.
	void start();
	//! Stops the listener thread and restores the terminal's original settings.
	void stop();

	//
	// Private methods.
	//
private:
	//! Thread body: polls stdin and toggles pause on any keypress.
	void run();

	//
	// Private data members.
	//
private:
	//! Toggled on keypress.
	const SignalController& signals_;
	//! Terminal settings as they were before start(), restored by stop().
	struct termios originalTermios_
	{
	};
	//! Whether the listener thread should keep running.
	std::atomic<bool> running_{ false };
	//! Background listener thread.
	std::thread thread_;
};

}	 // namespace common
