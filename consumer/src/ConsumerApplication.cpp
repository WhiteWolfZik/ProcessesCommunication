#include "ConsumerApplication.h"

#include <chrono>
#include <span>
#include <string>
#include <thread>
#include <utility>

#include "RingBufferLayout.h"

namespace consumer
{

namespace
{

constexpr auto pausePollInterval{ std::chrono::milliseconds(50) };

}	 // namespace

ConsumerApplication::ConsumerApplication(CliOptions options)
	: options_{ std::move(options) }
	, keypress_{ signals_ }
	, reporterThread_{ stats_, signals_, options_.reportInterval() }
{
}

int ConsumerApplication::run()
{
	signals_.install();
	keypress_.start();
	if (!reader_.attach(std::string{ common::ringBufferLayout::segmentName }, signals_))
	{
		keypress_.stop();
		return 0;
	}
	reporterThread_.start();

	while (!signals_.isStopRequested())
	{
		if (signals_.isPaused())
		{
			std::this_thread::sleep_for(pausePollInterval);
			continue;
		}

		if (!reader_.tryConsume(packet_))
		{
			continue;
		}

		const auto result{ validator_.validate(
			packet_.header, std::span<const std::uint8_t>(packet_.payload)) };
		stats_.record(packet_.header.payloadSize, result == ValidationResult::Valid);
	}

	reporterThread_.stop();
	keypress_.stop();
	return 0;
}

}	 // namespace consumer
