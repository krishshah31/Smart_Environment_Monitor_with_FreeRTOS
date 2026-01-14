/*
 * env_types.h
 *
 *  Created on: 14-Nov-2025
 *      Author: Krish
 */

#ifndef ENV_TYPES_H
#define ENV_TYPES_H

#include <stdint.h>

typedef enum
{
    SENSOR_OK = 0,
    SENSOR_ERR_TIMEOUT,
    SENSOR_ERR_CRC,
    SENSOR_ERR_UNKNOWN
} sensor_status_t;

typedef struct
{
    int16_t temperature_c;
    uint16_t humidity_pct;
    uint32_t timestamp_ms;
    sensor_status_t status;
} sensor_msg_t;

#endif // ENV_TYPES_H
