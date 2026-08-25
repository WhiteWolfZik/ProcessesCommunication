#include "ChecksumCalculator.h"

#include <array>
#include <cstdint>

namespace common
{

namespace crc32
{

constexpr auto tableSize{ 256U };
constexpr auto bitsPerByte{ 8 };
constexpr auto oneBit{ 1U };
constexpr auto byteMask{ 0xFFU };
constexpr auto polynomial{ 0xEDB88320U };
constexpr auto initialValue{ 0xFFFFFFFFU };

}	 // namespace crc32

namespace
{

std::array<std::uint32_t, crc32::tableSize> makeCrcTable()
{
	std::array<std::uint32_t, crc32::tableSize> table{};
	for (std::uint32_t i{ 0 }; i < crc32::tableSize; ++i)
	{
		std::uint32_t crc{ i };
		for (std::int32_t bit{ 0 }; bit < crc32::bitsPerByte; ++bit)
		{
			crc = (crc & crc32::oneBit) ? (crc >> crc32::oneBit) ^ crc32::polynomial
										: (crc >> crc32::oneBit);
		}
		table[i] = crc;
	}
	return table;
}

std::uint32_t crc32Update(std::uint32_t crc, const std::span<const std::byte> data)
{
	static const std::array<std::uint32_t, crc32::tableSize> table{ makeCrcTable() };
	for (const auto byte : data)
	{
		crc = table[(crc ^ std::to_integer<std::uint32_t>(byte)) & crc32::byteMask]
			^ (crc >> crc32::bitsPerByte);
	}
	return crc;
}

}	 // namespace

// Header fields are hashed individually rather than sizeof(header) so the
// (as yet unset) checksum field never feeds back into its own computation.
std::uint32_t ChecksumCalculator::compute(
	const PacketHeader& header, const std::span<const std::byte> payload)
{
	std::uint32_t crc{ crc32::initialValue };
	crc = crc32Update(crc, std::as_bytes(std::span(&header.sequenceNumber, 1)));
	crc = crc32Update(crc, std::as_bytes(std::span(&header.timestampNs, 1)));
	crc = crc32Update(crc, std::as_bytes(std::span(&header.payloadSize, 1)));
	crc = crc32Update(crc, payload);
	return crc ^ crc32::initialValue;
}

}	 // namespace common
