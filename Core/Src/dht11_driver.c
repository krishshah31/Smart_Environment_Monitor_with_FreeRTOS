/*
 * dht11_driver.c
 *
 *  Created on: 14-Nov-2025
 *      Author: Krish
 */

#include "dht11_driver.h"
#include "timebase_us.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

/* Helpers for fast GPIO mode switch */
static inline uint32_t pin_index_from_mask(uint32_t pinMask)
{
    /* Convert (1<<n) to n */
    uint32_t idx = 0;
    while ((pinMask >> idx) != 1U) { idx++; }
    return idx;
}

static inline void dht_pin_output(void)
{
    const uint32_t pin = pin_index_from_mask(DHT11_GPIO_PIN);
    /* MODER: 00=input, 01=output */
    DHT11_GPIO_PORT->MODER &= ~(3U << (pin * 2U));
    DHT11_GPIO_PORT->MODER |=  (1U << (pin * 2U));
}

static inline void dht_pin_input(void)
{
    const uint32_t pin = pin_index_from_mask(DHT11_GPIO_PIN);
    DHT11_GPIO_PORT->MODER &= ~(3U << (pin * 2U));
}

static inline void dht_write_low(void)
{
    /* BSRR reset: bit (pin+16) */
    DHT11_GPIO_PORT->BSRR = (DHT11_GPIO_PIN << 16);
}

static inline void dht_write_high(void)
{
    /* release line high (if push-pull). If you use external pullup + open-drain, driving high is ok too */
    DHT11_GPIO_PORT->BSRR = DHT11_GPIO_PIN;
}

static inline uint8_t dht_read_pin(void)
{
    return (DHT11_GPIO_PORT->IDR & DHT11_GPIO_PIN) ? 1U : 0U;
}

/* Wait until pin becomes expected level or timeout in us.
 * Returns 1 on success, 0 on timeout. */
static uint8_t wait_level(uint8_t level, uint32_t timeout_us)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t timeout_cycles = timeout_us * (SystemCoreClock / 1000000U);

    while (dht_read_pin() != level)
    {
        if ((DWT->CYCCNT - start) > timeout_cycles)
            return 0U;
    }
    return 1U;
}

/* Measure how long pin stays at 'level' (us), up to timeout_us.
 * Returns measured duration in us, or 0 on timeout/fail. */
static uint32_t measure_level_us(uint8_t level, uint32_t timeout_us)
{
    const uint32_t start_wait = DWT->CYCCNT;
    const uint32_t timeout_cycles = timeout_us * (SystemCoreClock / 1000000U);

    while (dht_read_pin() != level)
    {
        if ((DWT->CYCCNT - start_wait) > timeout_cycles)
            return 0U;
    }

    const uint32_t start = DWT->CYCCNT;
    while (dht_read_pin() == level)
    {
        if ((DWT->CYCCNT - start_wait) > timeout_cycles)
            return 0U;
    }

    const uint32_t cycles = (DWT->CYCCNT - start);
    return cycles / (SystemCoreClock / 1000000U);
}

void DHT11_Init(void)
{
    timebase_us_init();
    dht_pin_output();
    dht_write_high();
}

/* DHT11 protocol:
 * Start: pull low >= 18ms, then high 20-40us, then input.
 * Response: low ~80us, high ~80us.
 * Data: 40 bits: each bit low ~50us, then high 26-28us (0) or ~70us (1).
 */
sensor_status_t DHT11_Read(int16_t *temp_c, uint16_t *hum_pct)
{
    if (!temp_c || !hum_pct) return SENSOR_ERR_UNKNOWN;

    uint8_t data[5] = {0};

    /* Avoid preemption during critical timing window */
    taskENTER_CRITICAL();

    /* Start signal */
    dht_pin_output();
    dht_write_low();
    /* 18ms minimum */
    delay_us(18000);

    dht_write_high();
    delay_us(30);

    dht_pin_input();

    /* Sensor response */
    if (!wait_level(0, 100)) { taskEXIT_CRITICAL(); return SENSOR_ERR_TIMEOUT; }  // should go low
    if (!wait_level(1, 120)) { taskEXIT_CRITICAL(); return SENSOR_ERR_TIMEOUT; }  // then high
    if (!wait_level(0, 120)) { taskEXIT_CRITICAL(); return SENSOR_ERR_TIMEOUT; }  // then low to start data

    /* Read 40 bits */
    for (uint8_t i = 0; i < 40; i++)
    {
        /* Each bit starts with ~50us low */
        uint32_t low_us = measure_level_us(0, 120);
        if (low_us == 0) { taskEXIT_CRITICAL(); return SENSOR_ERR_TIMEOUT; }

        /* Then high: duration encodes bit */
        uint32_t high_us = measure_level_us(1, 150);
        if (high_us == 0) { taskEXIT_CRITICAL(); return SENSOR_ERR_TIMEOUT; }

        uint8_t bit = (high_us > 50) ? 1U : 0U;  // threshold between ~26us and ~70us

        data[i / 8] <<= 1;
        data[i / 8] |= bit;
    }

    taskEXIT_CRITICAL();

    /* Verify checksum */
    const uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4])
        return SENSOR_ERR_CRC;

    /* DHT11 gives integer values */
    *hum_pct = data[0];
    *temp_c  = data[2];

    return SENSOR_OK;
}

