
/*
 * @file        com.c
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

#include "com.h"

#define TAG "COM" 

/**********************************************************************************************************/
/**
 * @brief   Initializes the communication module.
 * @details This function sets up the necessary peripherals for communication (e.g., UART, I2C, SPI).
 *          It should be called before using any communication functions.
 * @return  None
 */
void com_init(void)
{
	/* install UART driver for console/default UART */
	uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 0, 0, 0, NULL, 0);
	ESP_LOGI(TAG, "initialized.");
}   
