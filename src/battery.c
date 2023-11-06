#include <stdint.h>
#include "tl_common.h"
#include "battery.h"

/* 2200..2950 mv - 0..100% or level in zigbee sepulkas */
uint8_t get_battery_level_in_zigbee_sepulkas(uint16_t battery_mv) {
	uint16_t battery_level = 0;
	if (battery_mv > MIN_VBAT_MV) {
		battery_level = (battery_mv - MIN_VBAT_MV) / ((MAX_VBAT_MV
				- MIN_VBAT_MV) / 50);
		if (battery_level > 200)
			battery_level = 200;
	}
	return (uint8_t)battery_level;
}
