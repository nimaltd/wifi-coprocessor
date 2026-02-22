
/*
 * @file        com.h
 * @brief       communication module for the ESP32 HOST project.
 * @author      Nima Askari
 * @version     1.0.0
 * @license     See the LICENSE file in the root folder.
 *
 * @note        All my libraries are dual-licensed. 
 *              Please review the licensing terms before using them.
 *              For any inquiries, feel free to contact me.
 *
 * @github      https://www.github.com/nimaltd
 * @linkedin    https://www.linkedin.com/in/nimaltd
 * @youtube     https://www.youtube.com/@nimaltd
 * @instagram   https://instagram.com/github.nimaltd
 *
 * Copyright (C) 2026 Nima Askari - NimaLTD. All rights reserved.
 */

#ifndef _COM_H_
#define _COM_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#if CONFIG_COM_UART_ENABLE
#include "driver/uart.h"
#endif
#if CONFIG_COM_SPI_ENABLE
#include "driver/spi_slave.h"  
#endif
#if CONFIG_COM_SDIO_ENABLE
#include "driver/sdio_slave.h"
#endif
#if CONFIG_COM_QSPI_ENABLE
#include "driver/qspi_slave.h"  
#endif

#ifdef __cplusplus
extern "C" {    
#endif

#define COM_FRAME_SIZE          2048
#define COM_FRAME_QUEUE_SIZE    10

void com_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _COM_H_ */