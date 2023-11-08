#include "crc.h"


Crc32::Crc32()
{
	for (uint32_t table_idx = 0; table_idx < 256; ++table_idx)
	{
		uint32_t crc = table_idx;

		for (size_t bit = 0; bit < 8; ++bit)
		{
			if (crc & 1) {
				crc = CRC32_POLYNOMIAL ^ (crc >> 1);
			}
			else {
				crc >>= 1;
			}
		}

		table[table_idx] = crc;
	}
}

uint32_t Crc32::calculate(const char* message, size_t message_size)
{
	uint32_t crc = CRC32_INITIAL;

	for (size_t i = 0; i < message_size; ++i)
	{
		crc = table[(crc ^ message[i]) & 0xFF] ^ (crc >> 8);
	}

	return ~crc;
}