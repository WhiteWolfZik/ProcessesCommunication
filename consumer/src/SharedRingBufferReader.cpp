#include "SharedRingBufferReader.h"

namespace consumer
{

void SharedRingBufferReader::attach([[maybe_unused]] const std::string& name)
{
	// TODO: shm_open (read-only) + mmap; retry with backoff until the
	// segment created by Producer exists.
}

std::optional<Packet> SharedRingBufferReader::tryConsume()
{
	// TODO: seqlock-versioned read at readIndex; FutexGate::wait() when the
	// buffer is empty (readIndex == writeIndex).
	return std::nullopt;
}

}	 // namespace consumer
