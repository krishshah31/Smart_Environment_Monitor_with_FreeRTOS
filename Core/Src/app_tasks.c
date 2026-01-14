/*
 * app_tasks.c
 *
 *  Created on: 20-Dec-2025
 *      Author: Krish
 */

#include "app_tasks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "env_types.h"
#include "dht11_driver.h"
#include "ssd1306_driver.h"
#include "uart_log.h"
#include "timebase_us.h"

#include "main.h"
#include <stdio.h>

// Queue handle
static QueueHandle_t xSensorQueue = NULL;

// Task prototypes
static void SensorTask(void *argument);
static void DisplayTask(void *argument);
static void UartLogTask(void *argument);
static void AlertTask(void *argument);

// Simple config
#define SENSOR_QUEUE_LEN     (1U)

#define SENSOR_TASK_STACK    (256U)
#define DISPLAY_TASK_STACK   (256U)
#define UART_TASK_STACK      (256U)
#define ALERT_TASK_STACK     (256U)

#define SENSOR_TASK_PRIO     (tskIDLE_PRIORITY + 3)
#define DISPLAY_TASK_PRIO    (tskIDLE_PRIORITY + 2)
#define UART_TASK_PRIO       (tskIDLE_PRIORITY + 1)
#define ALERT_TASK_PRIO      (tskIDLE_PRIORITY + 2)

void APP_Init(void)
{
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LEN, sizeof(sensor_msg_t));

    timebase_us_init();
    DHT11_Init();
    OLED_Init();
    UART_Log_Init();

    (void)xTaskCreate(SensorTask,  "SensorTask",  SENSOR_TASK_STACK,  NULL, SENSOR_TASK_PRIO,  NULL);
    (void)xTaskCreate(DisplayTask, "DisplayTask", DISPLAY_TASK_STACK, NULL, DISPLAY_TASK_PRIO, NULL);
    (void)xTaskCreate(UartLogTask, "UartLogTask", UART_TASK_STACK,    NULL, UART_TASK_PRIO,    NULL);
    (void)xTaskCreate(AlertTask,   "AlertTask",   ALERT_TASK_STACK,   NULL, ALERT_TASK_PRIO,   NULL);
}

static void SensorTask(void *argument)
{
    (void)argument;

    const TickType_t period = pdMS_TO_TICKS(2000);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        sensor_msg_t msg = {0};

        int16_t temp_c = 0;
        uint16_t hum_pct = 0;
        sensor_status_t st = DHT11_Read(&temp_c, &hum_pct);

        msg.temperature_c = temp_c;
        msg.humidity_pct  = hum_pct;
        msg.timestamp_ms  = HAL_GetTick();
        msg.status        = st;

        // queue length = 1: replace older sample if full
        if (xQueueSend(xSensorQueue, &msg, 0) != pdPASS)
        {
            sensor_msg_t throwaway;
            (void)xQueueReceive(xSensorQueue, &throwaway, 0);
            (void)xQueueSend(xSensorQueue, &msg, 0);
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void DisplayTask(void *argument)
{
    (void)argument;

    sensor_msg_t msg = {0};
    for (;;)
    {
        if (xQueueReceive(xSensorQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            char line1[32];
            char line2[32];

            if (msg.status == SENSOR_OK)
            {
                (void)snprintf(line1, sizeof(line1), "Temp: %d C", (int)msg.temperature_c);
                (void)snprintf(line2, sizeof(line2), "Hum : %u %%", (unsigned)msg.humidity_pct);
            }
            else
            {
                (void)snprintf(line1, sizeof(line1), "Sensor Error");
                (void)snprintf(line2, sizeof(line2), "Status: %d", (int)msg.status);
            }

            OLED_Clear();
            OLED_PrintLine(0, line1);
            OLED_PrintLine(1, line2);
            OLED_Update();
        }
    }
}

static void UartLogTask(void *argument)
{
    (void)argument;

    const TickType_t period = pdMS_TO_TICKS(2000);
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        sensor_msg_t latest = {0};

        if (xQueuePeek(xSensorQueue, &latest, 0) == pdPASS)
        {
            if (latest.status == SENSOR_OK)
            {
                UART_Log_Printf("[t=%lu] T=%dC H=%u%%\r\n",
                                (unsigned long)latest.timestamp_ms,
                                (int)latest.temperature_c,
                                (unsigned)latest.humidity_pct);
            }
            else
            {
                UART_Log_Printf("[t=%lu] Sensor error=%d\r\n",
                                (unsigned long)latest.timestamp_ms,
                                (int)latest.status);
            }
        }
        else
        {
            UART_Log_Printf("Waiting for first sensor reading...\r\n");
        }

        vTaskDelayUntil(&lastWake, period);
    }
}

static void AlertTask(void *argument)
{
    (void)argument;

    const int16_t TEMP_WARN_C = 30;
    const uint16_t HUM_WARN_PCT = 70;

    for (;;)
    {
        sensor_msg_t latest = {0};

        if (xQueuePeek(xSensorQueue, &latest, pdMS_TO_TICKS(100)) == pdPASS)
        {
            if (latest.status == SENSOR_OK &&
                (latest.temperature_c >= TEMP_WARN_C || latest.humidity_pct >= HUM_WARN_PCT))
            {
                UART_Log_Printf("ALERT! T=%dC H=%u%%\r\n",
                                (int)latest.temperature_c,
                                (unsigned)latest.humidity_pct);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

