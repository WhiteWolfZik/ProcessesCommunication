#include "ProducerApplication.h"

#include <iostream>
#include <utility>

namespace producer
{

ProducerApplication::ProducerApplication(CliOptions options)
	: options_{ std::move(options) }
{
}

int ProducerApplication::run()
{
	// TODO: signals_.install(); buffer_.open(...); main loop — generate
	// payload, build PacketHeader, compute checksum, publish, honor pause.
	std::cout << "producer: not implemented yet\n";
	return 0;
}

}	 // namespace producer
