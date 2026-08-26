#include "StatsReporterThread.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace consumer
{

namespace
{

constexpr auto wakePollInterval{ std::chrono::milliseconds(100) };

}	 // namespace

StatsReporterThread::StatsReporterThread(
	const StatsCollector& collector,
	const common::SignalController& signals,
	const std::chrono::seconds interval)
	: collector_{ collector }
	, signals_{ signals }
	, interval_{ interval }
{
}

StatsReporterThread::~StatsReporterThread()
{
	stop();
}

void StatsReporterThread::start()
{
	running_.store(true, std::memory_order_release);
	thread_ = std::thread(&StatsReporterThread::run, this);
}

void StatsReporterThread::stop()
{
	if (running_.exchange(false, std::memory_order_acq_rel) && thread_.joinable())
	{
		thread_.join();
	}
}

void StatsReporterThread::run()
{
	while (running_.load(std::memory_order_acquire))
	{
		auto remaining{ std::chrono::duration_cast<std::chrono::milliseconds>(interval_) };
		while (remaining > std::chrono::milliseconds::zero()
			   && running_.load(std::memory_order_acquire))
		{
			const auto step{ std::min(remaining, wakePollInterval) };
			std::this_thread::sleep_for(step);
			remaining -= step;
		}

		if (!running_.load(std::memory_order_acquire) || signals_.isPaused())
		{
			continue;
		}

		const StatsSnapshot current{ collector_.snapshot() };
		const auto packetsPerSec{ (current.totalPackets - lastSnapshot_.totalPackets)
								  / interval_.count() };
		const auto bytesPerSec{ (current.totalBytes - lastSnapshot_.totalBytes)
								/ interval_.count() };

		std::cout << "total=" << current.totalPackets << " invalid=" << current.invalidPackets
				  << " packets/sec=" << packetsPerSec << " bytes/sec=" << bytesPerSec << '\n';
		std::cout.flush();

		lastSnapshot_ = current;
	}
}

}	 // namespace consumer
