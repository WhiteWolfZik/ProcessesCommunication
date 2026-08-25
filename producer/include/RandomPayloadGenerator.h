#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace producer
{

class RandomPayloadGenerator
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	RandomPayloadGenerator();

	//
	// Public interface.
	//
public:
	//! Returns a buffer of size pseudo-random bytes.
	std::vector<std::uint8_t> generate(const std::size_t size);

	//
	// Private data members.
	//
private:
	//! Random number engine, seeded once at construction.
	std::mt19937_64 rng_;
	//! Uniform distribution over a single byte's value range.
	std::uniform_int_distribution<int> byteDistribution_;
};

}	 // namespace producer
