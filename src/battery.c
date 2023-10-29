#include <stdint.h>
#include "tl_common.h"
#include "battery.h"

// 2200..2950 mv - 0..100%
uint8_t get_battery_level(uint16_t battery_mv) {
	uint8_t battery_level = 0;
	if (battery_mv > MIN_VBAT_MV) {
		battery_level = (battery_mv - MIN_VBAT_MV) / ((MAX_VBAT_MV
				- MIN_VBAT_MV) / 100);
		if (battery_level > 100)
			battery_level = 100;
	}
	return battery_level;
}
