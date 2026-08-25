#pragma once

#include <atomic>
#include <chrono>
#include <thread>

#include "SignalController.h"
#include "StatsCollector.h"

namespace consumer
{

class StatsReporterThread
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	StatsReporterThread(
		const StatsCollector& collector,
		const common::SignalController& signals,
		const std::chrono::seconds interval);
	//! Destructor.
	~StatsReporterThread();
	//! Copy constructor.
	StatsReporterThread(const StatsReporterThread&) = delete;
	//! Copy assignment operator.
	StatsReporterThread& operator=(const StatsReporterThread&) = delete;

	//
	// Public interface.
	//
public:
	//! Starts the background reporting thread.
	void start();
	//! Signals the thread to stop and joins it.
	void stop();

	//
	// Private methods.
	//
private:
	//! Thread body: sleeps, then prints a statistics line unless paused.
	void run();

	//
	// Private data members.
	//
private:
	//! Counters to report on; only ever read via snapshot().
	const StatsCollector& collector_;
	//! Pause state to honor; only ever read via isPaused().
	const common::SignalController& signals_;
	//! Reporting period.
	std::chrono::seconds interval_;
	//! Whether the thread should keep running.
	std::atomic<bool> running_{ false };
	//! Background reporting thread.
	std::thread thread_;
	//! Previous snapshot, used to compute per-interval deltas.
	StatsSnapshot lastSnapshot_{};
};

}	 // namespace consumer
