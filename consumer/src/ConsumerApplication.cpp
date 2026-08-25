#include "ConsumerApplication.h"

#include <iostream>
#include <utility>

namespace consumer
{

ConsumerApplication::ConsumerApplication(CliOptions options)
	: options_{ std::move(options) }
	, reporterThread_{ stats_, signals_, options_.reportInterval() }
{
}

int ConsumerApplication::run()
{
	// TODO: signals_.install(); reader_.attach(...); reporterThread_.start();
	// main loop — tryConsume, validate, record; reporterThread_.stop() on exit.
	std::cout << "consumer: not implemented yet\n";
	return 0;
}

}	 // namespace consumer
