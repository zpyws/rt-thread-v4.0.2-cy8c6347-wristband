

#ifndef SENSOR_KIONIX_KX022_H__
#define SENSOR_KIONIX_KX022_H__

#include "sensor.h"
#include "kx022.h"

#define KX022_ADDR_DEFAULT UINT8_C(0x1e)

int rt_hw_kx022_init(const char *name, struct rt_sensor_config *cfg);

#endif
