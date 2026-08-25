#include "StatsCollector.h"

namespace consumer
{

void StatsCollector::record(const std::uint64_t bytes, const bool valid)
{
	totalPackets_.fetch_add(1, std::memory_order_relaxed);
	totalBytes_.fetch_add(bytes, std::memory_order_relaxed);
	if (!valid)
	{
		invalidPackets_.fetch_add(1, std::memory_order_relaxed);
	}
}

StatsSnapshot StatsCollector::snapshot() const
{
	return StatsSnapshot{
		totalPackets_.load(std::memory_order_relaxed),
		totalBytes_.load(std::memory_order_relaxed),
		invalidPackets_.load(std::memory_order_relaxed),
	};
}

}	 // namespace consumer
