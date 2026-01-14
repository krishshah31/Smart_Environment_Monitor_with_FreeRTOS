/*
 * dht11_driver.h
 *
 *  Created on: 14-Nov-2025
 *      Author: Krish
 */

#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include <stdint.h>
#include "env_types.h"

void DHT11_Init(void);
sensor_status_t DHT11_Read(int16_t *temp_c, uint16_t *hum_pct);

#endif // DHT11_DRIVER_H

