/*
 * ssd1306_driver.h
 *
 *  Created on: 15-Nov-2025
 *      Author: Krish
 */

#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdint.h>

void OLED_Init(void);
void OLED_Clear(void);
void OLED_PrintLine(uint8_t line, const char *text);
void OLED_Update(void);

#endif // SSD1306_DRIVER_H
