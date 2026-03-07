
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

#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "transport_layer.h"
#include <string.h>

#define TAG "MAIN"

/*
 * ********************************************************************************************************
 * CALLBACK IMPLEMENTATIONS
 * ********************************************************************************************************
*/

void app_main(void)
{
   /* install UART driver for console/default UART */
	uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 256, 0, NULL, 0);

   printf("\n========================================\n");
   printf("ESP32 WiFi Coprocessor - Version %s\n", PROJECT_VER);
   printf("https://www.github.com/nimaltd\n");
   printf("========================================\n\n");

   /* Initialize transport layer */
   tpl_init();
   
   ESP_LOGI(TAG, "Transport layer initialized");
   ESP_LOGI(TAG, "Slave ready - waiting for master commands");
}
