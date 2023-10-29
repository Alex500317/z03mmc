#pragma once

#include <stdint.h>

#define MAX_VBAT_MV		2950 // 3100 mV - > battery = 100% no load, 2950 at load (during measurement)
#define MIN_VBAT_MV		2200 // 2200 mV - > battery = 0%

#define END_VBAT_MV		2000 // It is not recommended to write Flash below 2V, go to deep-sleep

uint8_t get_battery_level(uint16_t battery_mv);

