#include "crc.h"

static uint32_t crc32_table[256];

void crc_32_init()
{
	for (uint32_t table_idx = 0; table_idx < 256; ++table_idx)
	{
		uint32_t c = table_idx;

		for (size_t j = 0; j < 8; ++j)
		{
			if (c & 1) {
				c = CRC32_POLYNOMIAL ^ (c >> 1);
			}
			else {
				c >>= 1;
			}
		}

		crc32_table[table_idx] = c;
	}
}

uint32_t crc_32(const char* message, size_t message_size)
{
	uint32_t c = 0 ^ 0xFFFFFFFF;
	const uint8_t* u = reinterpret_cast<const uint8_t*>(message);
	for (size_t i = 0; i < message_size; ++i)
	{
		c = crc32_table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}