#include "RandomPayloadGenerator.h"

namespace producer
{

RandomPayloadGenerator::RandomPayloadGenerator()
	: rng_{ std::random_device{}() }
	, byteDistribution_{ 0, 255 }
{
}

std::vector<std::uint8_t> RandomPayloadGenerator::generate(const std::size_t size)
{
	std::vector<std::uint8_t> payload(size);
	for (auto& byte : payload)
	{
		byte = static_cast<std::uint8_t>(byteDistribution_(rng_));
	}
	return payload;
}

}	 // namespace producer
