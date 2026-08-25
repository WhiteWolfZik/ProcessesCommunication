#pragma once

#include <atomic>
#include <cstdint>

namespace consumer
{

//! Point-in-time copy of StatsCollector's counters.
struct StatsSnapshot
{
	//! Total packets received.
	std::uint64_t totalPackets{ 0 };
	//! Total bytes received.
	std::uint64_t totalBytes{ 0 };
	//! Total corrupted or out-of-sequence packets.
	std::uint64_t invalidPackets{ 0 };
};

//! Counters are atomic because both the main receiving thread (record) and
//! StatsReporterThread (snapshot) touch this class concurrently.
class StatsCollector
{
	//
	// Public interface.
	//
public:
	//! Records one received packet's outcome.
	void record(const std::uint64_t bytes, const bool valid);
	//! Returns a consistent snapshot of the current counters.
	StatsSnapshot snapshot() const;

	//
	// Private data members.
	//
private:
	//! Total packets received.
	std::atomic<std::uint64_t> totalPackets_{ 0 };
	//! Total bytes received.
	std::atomic<std::uint64_t> totalBytes_{ 0 };
	//! Total corrupted or out-of-sequence packets.
	std::atomic<std::uint64_t> invalidPackets_{ 0 };
};

}	 // namespace consumer
