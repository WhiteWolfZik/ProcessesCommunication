#include "KeypressListener.h"

#include <poll.h>
#include <unistd.h>

#include <array>
#include <cstdint>

#include "SignalController.h"

namespace common
{

KeypressListener::KeypressListener(const SignalController& signals)
	: signals_(signals)
{
}

KeypressListener::~KeypressListener()
{
	stop();
}

void KeypressListener::start()
{
	if (::tcgetattr(STDIN_FILENO, &originalTermios_) != 0)
	{
		return;
	}

	struct termios raw
	{
		originalTermios_
	};
	raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
	if (::tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0)
	{
		return;
	}

	running_.store(true, std::memory_order_release);
	thread_ = std::thread(&KeypressListener::run, this);
}

void KeypressListener::stop()
{
	if (!running_.exchange(false, std::memory_order_acq_rel))
	{
		return;
	}
	if (thread_.joinable())
	{
		thread_.join();
	}
	::tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
}

void KeypressListener::run()
{
	std::array<struct pollfd, 1> pollFds{ { { STDIN_FILENO, POLLIN, 0 } } };

	while (running_.load(std::memory_order_acquire))
	{
		const auto result{ ::poll(
			pollFds.data(),
			pollFds.size(),
			static_cast<std::int32_t>(keypress::pollTimeout.count())) };
		if (result <= 0 || (pollFds[0].revents & POLLIN) == 0)
		{
			continue;
		}

		std::uint8_t keyBuffer{};
		if (::read(STDIN_FILENO, &keyBuffer, sizeof(keyBuffer)) > 0)
		{
			signals_.togglePause();
		}
	}
}

}	 // namespace common
