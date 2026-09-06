#pragma once

#include <cstdint>
#include <random>
#include <span>

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
	//! Fills out with pseudo-random bytes.
	void generate(const std::span<std::uint8_t> out);

	//
	// Private data members.
	//
private:
	//! Random number engine, seeded once at construction.
	std::mt19937_64 rng_;
};

}	 // namespace producer
