#pragma once

#include <cstdint>

#include "CliOptions.h"
#include "RandomPayloadGenerator.h"
#include "SharedRingBuffer.h"
#include "SignalController.h"

namespace producer
{

class ProducerApplication
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	explicit ProducerApplication(CliOptions options);

	//
	// Public interface.
	//
public:
	//! Runs the application until termination; returns the process exit code.
	int run();

	//
	// Private data members.
	//
private:
	//! Parsed command-line options.
	CliOptions options_;
	//! Produces packet payloads.
	RandomPayloadGenerator generator_;
	//! Shared-memory ring buffer publisher.
	SharedRingBuffer buffer_;
	//! Pause/resume signal handling.
	common::SignalController signals_;
	//! Next sequence number to assign to a published packet.
	std::uint64_t sequenceCounter_{ 0 };
};

}	 // namespace producer
