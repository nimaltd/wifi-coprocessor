
/*
 * @file        main.c
 * @brief       main application entry point for the ESP32 HOST project.
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

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "tpl.h"

/*
 * *********************************************************************************************************
 * Definitions
 * *********************************************************************************************************
*/

#define TAG "MAIN"

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
*/

void app_main(void)
{
   /* install UART driver for console/default UART */
	uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 256, 0, NULL, 0);

   ESP_LOGI(TAG, "========================================");
   ESP_LOGI(TAG, "ESP32 WiFi Coprocessor - Version %s", PROJECT_VER);
   ESP_LOGI(TAG, "https://www.github.com/nimaltd");
   ESP_LOGI(TAG, "========================================");

   /* Initialize transport layer */
   tpl_init();
}
