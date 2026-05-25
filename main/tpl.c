/*
 * @file        tpl.c
 * @brief       WiFi Co-Processor Transport Layer
 * @author      Nima Askari
 * @version     0.0.1
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "tpl.h"
#include "tpl_spi.h"
#include "tpl_usart.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

#define TAG "TPL"

/*
 * *********************************************************************************************************
 * Global Variables
 * *********************************************************************************************************
*/


/*
 * *********************************************************************************************************
 * Private Function Prototypes
 * *********************************************************************************************************
*/

void tpl_task(void *parameters);

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
*/

void tpl_init(void)
{
    /* LED pin (output, idle off) */
    gpio_config_t io =
    {
        .pin_bit_mask = (1ULL << CONFIG_LED_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    gpio_set_level(CONFIG_LED_PIN, !CONFIG_LED_ACTIVE_LEVEL);

    /* Event pin (output, idle low) */
    io.pin_bit_mask = (1ULL << CONFIG_EVENT_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_EVENT_PIN, 0);

    /* READY pin (output, idle low) */
    io.pin_bit_mask = (1ULL << CONFIG_READY_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_READY_PIN, 0);

    tpl_spi_init();
    tpl_usart_init();

    xTaskCreate(tpl_task, "tpl_task", TPL_TASK_STACK_SIZE, NULL, TPL_TASK_PRIORITY, NULL);
}

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief   Main task for the Transport Layer. This task will handle all the communication with the host.
 *          It will read commands from the host, execute them, and send back responses.
 */
void tpl_task(void *parameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

