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
	reader_.attach(std::string{ common::ringBufferLayout::segmentName });
	reporterThread_.start();

	while (!signals_.isStopRequested())
	{
		if (signals_.isPaused())
		{
			std::this_thread::sleep_for(pausePollInterval);
			continue;
		}

		auto packet{ reader_.tryConsume() };
		if (!packet)
		{
			continue;
		}

		const auto result{ validator_.validate(
			packet->header, std::span<const std::uint8_t>(packet->payload)) };
		stats_.record(packet->header.payloadSize, result == ValidationResult::Valid);
	}

	reporterThread_.stop();
	keypress_.stop();
	return 0;
}

}	 // namespace consumer
