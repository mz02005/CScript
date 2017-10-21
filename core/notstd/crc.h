#pragma once
#include "config.h"

namespace notstd {
	NOTSTD_API uint16_t CalcCRC16_1(uint16_t crc, const uint8_t *buff, uint32_t len);
}
