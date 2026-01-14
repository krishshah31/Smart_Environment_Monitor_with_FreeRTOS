/*
 * uart_log.c
 *
 *  Created on: 14-Nov-2025
 *      Author: Krish
 */

#include "uart_log.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void UART_Log_Init(void)
{
    // UART already initialized in MX_USARTx_UART_Init()
}

void UART_Log_Printf(const char *fmt, ...)
{
    char buf[160];

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n < 0) return;

    size_t len = strnlen(buf, sizeof(buf));
    (void)HAL_UART_Transmit(&huart2, (uint8_t*)buf, (uint16_t)len, 100);
}


