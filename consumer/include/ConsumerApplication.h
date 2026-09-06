#pragma once

#include "CliOptions.h"
#include "KeypressListener.h"
#include "PacketValidator.h"
#include "SharedRingBufferReader.h"
#include "SignalController.h"
#include "StatsCollector.h"
#include "StatsReporterThread.h"

namespace consumer
{

class ConsumerApplication
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	explicit ConsumerApplication(CliOptions options);

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
	//! Shared-memory ring buffer subscriber.
	SharedRingBufferReader reader_;
	//! Checksum and sequence validation.
	PacketValidator validator_;
	//! Received-packet counters.
	StatsCollector stats_;
	//! Pause/resume signal handling.
	common::SignalController signals_;
	//! Pause/resume via keypress, sharing signals_'s pause flag.
	common::KeypressListener keypress_;
	//! Background statistics printer.
	StatsReporterThread reporterThread_;
	//! Reused across tryConsume() calls to avoid a heap allocation per packet.
	Packet packet_;
};

}	 // namespace consumer
