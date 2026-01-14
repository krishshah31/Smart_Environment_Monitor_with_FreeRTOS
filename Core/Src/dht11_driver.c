/*
 * dht11_driver.c
 *
 *  Created on: 14-Nov-2025
 *      Author: Krish
 */

#include "dht11_driver.h"
#include "main.h"

void DHT11_Init(void)
{
    // GPIO init done by CubeMX; real protocol later
}

sensor_status_t DHT11_Read(int16_t *temp_c, uint16_t *hum_pct)
{
    if (!temp_c || !hum_pct) return SENSOR_ERR_UNKNOWN;

    uint32_t t = HAL_GetTick();
    *temp_c = (int16_t)(22 + (t / 1000) % 6);     // 22..27
    *hum_pct = (uint16_t)(45 + (t / 1000) % 10);  // 45..54
    return SENSOR_OK;
}

