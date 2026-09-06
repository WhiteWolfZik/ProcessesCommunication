#include "RandomPayloadGenerator.h"

#include <cstring>

namespace producer
{

RandomPayloadGenerator::RandomPayloadGenerator()
	: rng_{ std::random_device{}() }
{
}

void RandomPayloadGenerator::generate(const std::span<std::uint8_t> out)
{
	std::size_t written{ 0 };
	while (written + sizeof(std::uint64_t) <= out.size())
	{
		const std::uint64_t word{ rng_() };
		std::memcpy(out.data() + written, &word, sizeof(word));
		written += sizeof(word);
	}
	if (written < out.size())
	{
		const std::uint64_t word{ rng_() };
		std::memcpy(out.data() + written, &word, out.size() - written);
	}
}

}	 // namespace producer
