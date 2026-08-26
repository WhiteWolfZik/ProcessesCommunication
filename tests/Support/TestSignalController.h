#pragma once

#include <atomic>

#include "SignalController.h"

namespace tests
{

class TestSignalController : public common::SignalController
{
	//
	// Public interface.
	//
public:
	//! Resets the shared pause/stop-requested flags to their initial state.
	static void resetForTesting()
	{
		paused_.store(false, std::memory_order_release);
		stopRequested_.store(false, std::memory_order_release);
	}
};

}	 // namespace tests
