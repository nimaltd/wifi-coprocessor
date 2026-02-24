
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

static QueueHandle_t com_queue;

static void com_task(void *arg);

/**********************************************************************************************************/
/**
 * @brief   Initializes the communication module.
 * @details This function sets up the necessary peripherals for communication (e.g., UART, I2C, SPI).
 *          It should be called before using any communication functions.
 * @return  None
 */
void com_init(void)
{
    gpio_config_t io_conf = {};

	/* LED initialization */
	gpio_set_level(CONFIG_LED_PIN, !CONFIG_LED_ACTIVE_LEVEL);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_LED_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

	/* IRQ-OUT initialization */
	gpio_set_level(CONFIG_IRQ_OUT_PIN, !CONFIG_IRQ_OUT_ACTIVE_LEVEL);
	io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << CONFIG_IRQ_OUT_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

#if CONFIG_COM_UART
	/* UART initialization */
	uart_config_t uart_config =
	{
		.baud_rate = CONFIG_COM_UART_BAUD,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	ESP_ERROR_CHECK(uart_driver_install(CONFIG_COM_UART_NUM, COM_FRAME_SIZE, COM_FRAME_SIZE,
		COM_FRAME_QUEUE_SIZE, &com_queue, 0));
	ESP_ERROR_CHECK(uart_param_config(CONFIG_COM_UART_NUM, &uart_config));	
	ESP_ERROR_CHECK(uart_set_pin(CONFIG_COM_UART_NUM, CONFIG_COM_UART_TX_PIN, CONFIG_COM_UART_RX_PIN,
		UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	

#endif

#if CONFIG_COM_SPI
	/* SPI initialization */
	

#endif

#if CONFIG_COM_QSPI
	/* QSPI initialization */		

#endif

	xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, NULL);
	ESP_LOGI(TAG, "initialized.");
}   


static void com_task(void *arg)
{
	uint8_t *data = (uint8_t *) malloc(COM_FRAME_SIZE);
	while (1)
	{
		int len = uart_read_bytes(CONFIG_COM_UART_NUM, data, (COM_FRAME_SIZE - 1), 1);
		if (len > 0)
		{
			ESP_LOGI(TAG, "Received %d bytes", len);
		}
	}
}