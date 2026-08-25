#include <utility>

#include "CliOptions.h"
#include "ProducerApplication.h"

int main(int argc, char** argv)
{
	auto options = producer::CliOptions::parse(argc, argv);
	producer::ProducerApplication app(std::move(options));
	return app.run();
}
