#include <utility>

#include "CliOptions.h"
#include "ConsumerApplication.h"

int main(int argc, char** argv)
{
	auto options = consumer::CliOptions::parse(argc, argv);
	consumer::ConsumerApplication app(std::move(options));
	return app.run();
}
