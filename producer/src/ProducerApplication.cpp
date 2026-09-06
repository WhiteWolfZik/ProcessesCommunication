#include "ProducerApplication.h"

#include <chrono>
#include <span>
#include <string>
#include <thread>
#include <utility>

#include "ChecksumCalculator.h"
#include "RingBufferLayout.h"

namespace producer
{

namespace
{

constexpr auto pausePollInterval{ std::chrono::milliseconds(50) };

std::uint64_t nowNanoseconds()
{
	return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
										  std::chrono::system_clock::now().time_since_epoch())
										  .count());
}

}	 // namespace

ProducerApplication::ProducerApplication(CliOptions options)
	: options_{ std::move(options) }
	, keypress_{ signals_ }
{
}

int ProducerApplication::run()
{
	signals_.install();
	keypress_.start();
	buffer_.open(
		std::string{ common::ringBufferLayout::segmentName },
		options_.ringBufferBytes(),
		options_.payloadSize());
	payloadBuffer_.resize(options_.payloadSize());

	while (!signals_.isStopRequested())
	{
		if (signals_.isPaused())
		{
			std::this_thread::sleep_for(pausePollInterval);
			continue;
		}

		generator_.generate(std::span<std::uint8_t>(payloadBuffer_));

		common::PacketHeader header{};
		header.sequenceNumber = sequenceCounter_++;
		header.timestampNs = nowNanoseconds();
		header.payloadSize = static_cast<std::uint32_t>(payloadBuffer_.size());
		header.checksum = common::ChecksumCalculator::compute(
			header, std::as_bytes(std::span<const std::uint8_t>(payloadBuffer_)));

		buffer_.publish(header, std::span<const std::uint8_t>(payloadBuffer_));
	}

	keypress_.stop();
	return 0;
}

}	 // namespace producer
