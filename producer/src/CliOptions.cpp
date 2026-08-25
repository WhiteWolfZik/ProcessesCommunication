#include "CliOptions.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace producer
{

namespace
{

constexpr auto decimalRadix{ 10 };

[[noreturn]] void printUsageAndExit(const std::string_view programName)
{
	std::cerr << "Usage: " << programName << " <payloadSize> [--buffer-size BYTES]\n";
	std::exit(EXIT_FAILURE);
}

}	 // namespace

CliOptions CliOptions::parse(const int argc, char** argv)
{
	if (argc < 2)
	{
		printUsageAndExit(argv[0]);
	}

	CliOptions options;
	options.payloadSize_ = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, decimalRadix));
	if (options.payloadSize_ == 0)
	{
		printUsageAndExit(argv[0]);
	}

	for (std::int32_t i{ 2 }; i < argc; ++i)
	{
		const std::string_view arg{ argv[i] };
		if (arg == "--buffer-size" && i + 1 < argc)
		{
			options.ringBufferBytes_ = std::strtoull(argv[++i], nullptr, decimalRadix);
		}
		else
		{
			printUsageAndExit(argv[0]);
		}
	}

	return options;
}

}	 // namespace producer
