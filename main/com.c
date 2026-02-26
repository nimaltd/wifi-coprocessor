
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
#include "esp_err.h"
#include "driver/spi_common.h"
#include <string.h>

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

static com_t com = {0};

#define COM_SPI_HOST               SPI2_HOST
#define COM_SPI_QUEUE_SIZE         8

static uint8_t com_last_cmd = 0;
static uint8_t com_last_addr = 0;
static size_t com_last_len = 0;
static bool com_last_is_rx = false;

/* External event queue functions (implemented in com_queue.c) */
extern void com_event_queue_init(void);
extern bool com_event_pop(uint8_t *type, uint8_t *param, uint8_t *data, uint16_t *len);

/*
 * ********************************************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 * ********************************************************************************************************
*/

static void com_task(void *arg);
static void com_spi_slave_init(void);
static bool com_prepare_data_phase(uint8_t cmd, uint8_t addr, spi_slave_transaction_t *data_trans);

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

	/* SPI slave initialization (half-duplex capable, 1-bit/4-bit auto detect) */
	com_spi_slave_init();

	/* Initialize event queue */
	com_event_queue_init();

	
	/* To indicate initialization is complete */
	gpio_set_level(CONFIG_IRQ_OUT_PIN, 1);

	xTaskCreate(com_task, "com_task", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, NULL);
}   

/*
 * ********************************************************************************************************
 * WEAK CALLBACK IMPLEMENTATION
 * ********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief   RX event callback - invoked when data is received from master.
 * @details This weak function can be overridden by the user to handle received data.
 *          The default implementation does nothing.
 * @param   cmd: The command byte associated with the transaction
 * @param   addr: The address byte associated with the transaction
 * @param   data: Pointer to the received data buffer
 * @param   len: Length of the received data
 * @return  None
 */
__attribute__((weak)) void com_on_event_rx(uint8_t cmd, uint8_t addr, uint8_t *data, size_t len)
{
	(void)cmd;
	(void)addr;
	(void)data;
	(void)len;
	ESP_LOGD(TAG, "Received data from master: cmd=0x%02X, addr=0x%02X, len=%d", cmd, addr, len);	
}

/**********************************************************************************************************/
/**
 * @brief   TX event callback - invoked when data needs to be sent to master.
 * @details This weak function can be overridden by the user to fill the transmit buffer.
 *          The default implementation does nothing.
 * @param   cmd: The command byte associated with the transaction
 * @param   addr: The address byte associated with the transaction
 * @param   data: Pointer to the transmit buffer (to be filled by user)
 * @param   len: Maximum length available in the transmit buffer
 * @return  None
 */
__attribute__((weak)) void com_on_event_tx(uint8_t cmd, uint8_t addr, uint8_t *data, size_t len)
{
	(void)cmd;
	(void)addr;
	(void)data;
	(void)len;
	ESP_LOGD(TAG, "Preparing data for master: cmd=0x%02X, addr=0x%02X, max_len=%d", cmd, addr, len);
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
	com_cmd_hdr_t hdr = {0};
	spi_slave_transaction_t cmd_trans = {0};
	spi_slave_transaction_t data_trans = {0};
	spi_slave_transaction_t *result = NULL;

	while (1)
	{
		/* Queue command transaction */
		hdr.cmd = 0;
		hdr.addr = 0;
		cmd_trans.length = 16;
		cmd_trans.rx_buffer = &hdr;
		ESP_ERROR_CHECK(spi_slave_queue_trans(COM_SPI_HOST, &cmd_trans, portMAX_DELAY));

		/* Wait for command to complete */
		ESP_ERROR_CHECK(spi_slave_get_trans_result(COM_SPI_HOST, &result, portMAX_DELAY));

		/* Prepare and handle data phase if needed */
		if (com_prepare_data_phase(hdr.cmd, hdr.addr, &data_trans))
		{
			/* Queue data transaction */
			ESP_ERROR_CHECK(spi_slave_queue_trans(COM_SPI_HOST, &data_trans, portMAX_DELAY));

			/* Wait for data to complete */
			ESP_ERROR_CHECK(spi_slave_get_trans_result(COM_SPI_HOST, &result, portMAX_DELAY));

			/* Invoke callback for RX data */
			if (com_last_is_rx)
			{
				com_on_event_rx(com_last_cmd, com_last_addr, com.rx_data, com_last_len);
			}
			else
			{
				/* For TX, signal master that data has been sent */
				gpio_set_level(CONFIG_IRQ_OUT_PIN, 1);
			}
		}
	}
}

/**********************************************************************************************************/
/**
 * @brief   Initializes the SPI slave interface.
 * @details This function configures the SPI peripheral to operate in slave mode with the specified pin configuration and settings.
 *          It sets up the necessary GPIO pins for SPI communication and initializes the SPI driver.
 * @return  None
 */
static void com_spi_slave_init(void)
{
	spi_bus_config_t buscfg = 
	{
		.mosi_io_num = CONFIG_COM_SPI_D0_MOSI_PIN,
		.miso_io_num = CONFIG_COM_SPI_D1_MISO_PIN,
		.sclk_io_num = CONFIG_COM_SPI_SCK_PIN,
		.quadwp_io_num = CONFIG_COM_SPI_D2_PIN,
		.quadhd_io_num = CONFIG_COM_SPI_D3_PIN,
		.max_transfer_sz = COM_FRAME_SIZE,
		.flags = SPICOMMON_BUSFLAG_DUAL | SPICOMMON_BUSFLAG_QUAD  // Enable 2-bit and 4-bit modes
	};

	spi_slave_interface_config_t slvcfg = 
	{
		.spics_io_num = CONFIG_COM_SPI_CS_PIN,
		.flags = 0,
		.queue_size = COM_SPI_QUEUE_SIZE,
		.mode = 0
	};

	ESP_ERROR_CHECK(spi_slave_initialize(COM_SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));

	/* Enable pull-ups on all SPI pins to stabilize floating inputs when no master is connected */
	gpio_pullup_en(CONFIG_COM_SPI_D0_MOSI_PIN);
	gpio_pullup_en(CONFIG_COM_SPI_D1_MISO_PIN);
	gpio_pullup_en(CONFIG_COM_SPI_SCK_PIN);
	gpio_pullup_en(CONFIG_COM_SPI_CS_PIN);
	gpio_pullup_en(CONFIG_COM_SPI_D2_PIN);
	gpio_pullup_en(CONFIG_COM_SPI_D3_PIN);
}

/**********************************************************************************************************/
/**
 * @brief   Prepares the data phase for SPI communication based on the command and address.
 * @details This function configures the SPI transaction structure for the data phase, setting the appropriate
 *          flags and buffers based on the command and I/O mode.
 * @param   cmd: SPI command
 * @param   addr: SPI address
 * @param   data_trans: Pointer to the SPI transaction structure to be configured
 * @return  true if a data phase is required, false otherwise
 */
static bool com_prepare_data_phase(uint8_t cmd, uint8_t addr, spi_slave_transaction_t *data_trans)
{
	uint8_t base_cmd = cmd & COM_CMD_MASK_BASE;
	uint8_t io_mode = cmd & COM_CMD_MASK_IOMODE;

	*data_trans = (spi_slave_transaction_t){0};
	com_last_cmd = cmd;
	com_last_addr = addr;
	com_last_len = 0;
	com_last_is_rx = false;

	if (cmd == COM_CMD_EXQPI)
	{
		ESP_LOGD(TAG, "SPI command: EXQPI");
		return false;
	}

	if (base_cmd == COM_CMD_ENQPI)
	{
		ESP_LOGD(TAG, "SPI command: ENQPI");
		return false;
	}

	/* Track the requested IO mode (informational - hardware adapts automatically) */
	switch (io_mode)
	{
		case COM_IOMODE_DOUT:
		case COM_IOMODE_DIO:
			com.mode = COM_MODE_SPI;
			break;
		case COM_IOMODE_QOUT:
		case COM_IOMODE_QIO:
			com.mode = COM_MODE_QSPI;
			break;
		case COM_IOMODE_1BIT:
		default:
			com.mode = COM_MODE_SPI;
			break;
	}

	switch (base_cmd)
	{
		case COM_CMD_WRBUF:
		case COM_CMD_WRDMA:
			memset(com.rx_data, 0, sizeof(com.rx_data));
			com_last_len = COM_FRAME_SIZE;
			com_last_is_rx = true;
			data_trans->length = com_last_len * 8;
			data_trans->rx_buffer = com.rx_data;
			ESP_LOGD(TAG, "SPI command: WR (0x%02X), addr=0x%02X", cmd, addr);
			return true;

		case COM_CMD_RDBUF:
		case COM_CMD_RDDMA:
			com_last_len = COM_FRAME_SIZE;
			com_last_is_rx = false;
			
			/* Try to get event from queue */
			uint8_t event_type = 0;
			uint8_t event_param = 0;
			uint16_t event_len = 0;
			if (com_event_pop(&event_type, &event_param, com.tx_data + 4, &event_len))
			{
				/* Pack queued event into TX buffer: [type(1)][param(1)][len(2)][data] */
				com.tx_data[0] = event_type;
				com.tx_data[1] = event_param;
				com.tx_data[2] = (event_len >> 8) & 0xFF;
				com.tx_data[3] = event_len & 0xFF;
				com_last_len = 4 + event_len;
			}
			else
			{
				/* Queue empty, invoke callback for manual buffer fill */
				com_on_event_tx(cmd, addr, com.tx_data, COM_FRAME_SIZE);
				com_last_len = COM_FRAME_SIZE;
			}
			
			/* Signal master that data is ready */
			gpio_set_level(CONFIG_IRQ_OUT_PIN, 0);
			data_trans->length = com_last_len * 8;
			data_trans->tx_buffer = com.tx_data;
			ESP_LOGD(TAG, "SPI command: RD (0x%02X), addr=0x%02X", cmd, addr);
			return true;

		case COM_CMD_SEG_DONE:
			ESP_LOGD(TAG, "SPI command: SEG_DONE");
			return false;

		case COM_CMD_WR_DONE:
			ESP_LOGD(TAG, "SPI command: WR_DONE");
			return false;

		case COM_CMD_CMD8:
		case COM_CMD_CMD9:
		case COM_CMD_CMDA:
			ESP_LOGD(TAG, "SPI command: CMDx (0x%02X)", cmd);
			return false;

		default:
			ESP_LOGW(TAG, "SPI command: unknown (0x%02X)", cmd);
			return false;
	}
}

/*
 * ********************************************************************************************************
 * END
 * ********************************************************************************************************
*/