
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

/*
 * ********************************************************************************************************
 * INCLUDES
 * ********************************************************************************************************
*/

#include "com.h"

/*
 * ********************************************************************************************************
 * DEFINES
 * ********************************************************************************************************
*/

#define TAG "COM" 

/*
 * ********************************************************************************************************
 * VARIABLES
 * ********************************************************************************************************
*/

static QueueHandle_t com_queue;
static com_t com = {0};

/*
 * ********************************************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 * ********************************************************************************************************
*/

static void com_task(void *arg);

/*
 * ********************************************************************************************************
 * FUNCTIONS IMPLEMENTATION
 * ********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief   Initializes the communication module.
 * @details This function sets up the necessary peripherals for communication (e.g., UART, I2C, SPI).
 *          It should be called before using any communication functions.
 * @return  None
 */
void com_init(void)
{
    gpio_config_t io_conf = {0};

	/* LED PIN initialization */
	gpio_set_level(CONFIG_LED_PIN, !CONFIG_LED_ACTIVE_LEVEL);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_LED_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

	/* IRQ-OUT PIN initialization */
	gpio_set_level(CONFIG_IRQ_OUT_PIN, 0);
	io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_IRQ_OUT_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

	/* MODE PIN initialization */
	io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_MODE_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

	if (gpio_get_level(CONFIG_MODE_PIN) == 1)
	{
		
		ESP_LOGI(TAG, "Operating in SPI mode");
	}
	else
	{
		ESP_LOGI(TAG, "Operating in QSPI mode");
	}

	/* MODE PIN de-initialization */
	io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_MODE_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

	/* To indicate initialization is complete */
	gpio_set_level(CONFIG_IRQ_OUT_PIN, 1);

	xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, NULL);
	ESP_LOGI(TAG, "initialized.");
}   

/*
 * ********************************************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATION
 * ********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief   Communication task that handles incoming data and events.
 * @details This task runs in the background and processes communication events
 * 		such as receiving data from UART, SPI, or QSPI. It can also handle timeouts and other communication-related events.
 * @param   arg: Task argument (not used)
 * @return  None
 */
static void com_task(void *arg)
{
	
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}