#include "CliOptions.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace consumer
{

namespace
{

constexpr auto decimalRadix{ 10 };

[[noreturn]] void printUsageAndExit(const std::string_view programName)
{
	std::cerr << "Usage: " << programName << " [--interval SECONDS]\n";
	std::exit(EXIT_FAILURE);
}

}	 // namespace

CliOptions CliOptions::parse(const int argc, char** argv)
{
	CliOptions options;

	for (std::int32_t i{ 1 }; i < argc; ++i)
	{
		const std::string_view arg{ argv[i] };
		if (arg == "--interval" && i + 1 < argc)
		{
			options.reportInterval_ =
				std::chrono::seconds{ std::strtoul(argv[++i], nullptr, decimalRadix) };
		}
		else
		{
			printUsageAndExit(argv[0]);
		}
	}

	if (options.reportInterval_.count() == 0)
	{
		printUsageAndExit(argv[0]);
	}

	return options;
}

}	 // namespace consumer
