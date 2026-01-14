# Smart Environment Monitor with FreeRTOS (STM32F446RE)

A **real-time embedded system** implemented on **STM32F446RE** using **FreeRTOS**, featuring
multi-task architecture, real sensor communication, OLED display output, and UART logging.

This project runs on **real hardware** and demonstrates **RTOS-based system design**, 
task synchronization, and low-level driver integration.

---

## 🎯 Project Overview

The Smart Environment Monitor periodically measures environmental parameters and presents them through
multiple concurrent outputs using FreeRTOS tasks.

### Key features:
- Real FreeRTOS scheduler running on STM32
- Concurrent tasks with defined priorities
- Inter-task communication using FreeRTOS queues
- Real hardware drivers for sensor and display
- UART logging for debugging and monitoring

---

## 🧠 System Architecture

The firmware follows a **producer–consumer RTOS model**:

### Tasks
- **SensorTask**
  - Communicates with a DHT11 temperature & humidity sensor
  - Uses precise microsecond timing (DWT-based)
  - Publishes measurements to a FreeRTOS queue

- **DisplayTask**
  - Consumes sensor data
  - Updates SSD1306 OLED display via I2C

- **UartLogTask**
  - Logs real-time data to PC terminal via UART

- **AlertTask**
  - Monitors threshold conditions
  - Generates alert messages when limits are exceeded

### Communication
- Tasks exchange data using **FreeRTOS queues**
- No shared global data between tasks
- Blocking and non-blocking access used appropriately

---

## 🧱 Project Structure

```
Core/
├── Inc/
│   ├── app_tasks.h          // RTOS task interface
│   ├── env_types.h          // Shared data structures
│   ├── dht11_driver.h       // DHT11 sensor driver
│   ├── ssd1306_driver.h     // SSD1306 OLED driver
│   ├── uart_log.h           // UART logging interface
│   └── timebase_us.h        // Microsecond timing (DWT)
│
├── Src/
│   ├── app_tasks.c          // Task creation & scheduling
│   ├── dht11_driver.c       // Real DHT11 protocol implementation
│   ├── ssd1306_driver.c     // Real SSD1306 I2C display driver
│   ├── uart_log.c           // UART logging implementation
│   └── timebase_us.c        // DWT-based delay utilities
│
├── freertos.c               // CubeMX-generated RTOS bootstrap
├── main.c                   // System entry point
└── stm32f4xx_it.c           // Interrupt handlers
```


---

## 🔌 Hardware & Interfaces

- **MCU**: STM32F446RE
- **Sensor**: DHT11 (GPIO, single-wire protocol)
- **Display**: SSD1306 OLED (I2C, 128×64)
- **Logging**: UART (ST-Link VCP)
- **Timing**: DWT cycle counter for microsecond precision

---

## ⚙️ RTOS & Timing Design

- FreeRTOS integrated using **CMSIS-RTOS v2**
- HAL timebase moved to **TIM6** (RTOS-safe)
- DHT11 communication protected using critical sections
- Tasks designed to avoid blocking each other unnecessarily

---

## 🛠 Tools & Environment

- **IDE**: STM32CubeIDE
- **Configuration**: STM32CubeMX
- **RTOS**: FreeRTOS
- **Language**: C
- **Target Board**: STM32F446RE / Nucleo-F446RE

---

## 🚀 Current Status

- ✅ Project builds successfully
- ✅ Runs on real STM32 hardware
- ✅ FreeRTOS tasks scheduled correctly
- ✅ Real DHT11 sensor readings
- ✅ Real OLED display output
- ✅ UART logs visible on PC terminal

---

## 📌 Notes

This project was developed with a **system-first approach**:
RTOS architecture and task design were established before integrating
timing-critical peripheral drivers.

The result is a clean, extensible firmware structure suitable for
larger embedded systems.

---

## 📄 License

This project is provided for educational and portfolio purposes.


