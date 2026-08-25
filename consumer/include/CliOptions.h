#pragma once

#include <chrono>

namespace consumer
{

namespace defaults
{

constexpr std::chrono::seconds reportInterval{ 1 };

}	 // namespace defaults

class CliOptions
{
	//
	// Public interface.
	//
public:
	//! Parses argv into a CliOptions instance; exits the process on invalid input.
	static CliOptions parse(const int argc, char** argv);
	//! Returns the configured statistics reporting interval.
	std::chrono::seconds reportInterval() const
	{
		return reportInterval_;
	}

	//
	// Private data members.
	//
private:
	//! Reporting interval, from --interval (default 1 second).
	std::chrono::seconds reportInterval_{ defaults::reportInterval };
};

}	 // namespace consumer
