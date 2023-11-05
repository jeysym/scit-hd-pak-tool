#pragma once
#include "common.h"

static constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320;

void crc_32_init();
uint32_t crc_32(const char* message, size_t message_size);