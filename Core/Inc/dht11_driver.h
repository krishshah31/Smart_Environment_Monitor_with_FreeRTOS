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

/* ===== USER CONFIG =====
 * Set the DHT11 data pin here to match your wiring / CubeMX pin choice.
 */
#define DHT11_GPIO_PORT   GPIOA
#define DHT11_GPIO_PIN    (1U << 1)   /* Example: PA1 => bit1. Change if needed. */
/* ======================= */

void DHT11_Init(void);
sensor_status_t DHT11_Read(int16_t *temp_c, uint16_t *hum_pct);

#endif

