#include "io_driver.h"
#include <math.h>
#include <stdint.h>

int joystickpos_to_percentage(uint8_t voltage)
{
	return round(200/3 * (voltage - 2.5));
}

int touchpadpos_to_percentage(uint8_t voltage)
{
	return round(200/5 * (voltage - 2.5));
}