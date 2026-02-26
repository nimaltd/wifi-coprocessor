
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

#ifdef __cplusplus
extern "C" {    
#endif

/*
 * ********************************************************************************************************
 * INCLUDES
 * ********************************************************************************************************
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/spi_slave.h"  
#include "driver/spi_slave_hd.h"  

/*
 * ********************************************************************************************************
 * DEFINES
 * ********************************************************************************************************
*/

#define COM_FRAME_SIZE          2048
#define COM_FRAME_QUEUE_SIZE    10
#define COM_TASK_STACK_SIZE     4096
#define COM_TASK_PRIORITY       10
#define COM_EVENT_QUEUE_LEN     16

/*
 * ********************************************************************************************************
 * DEFINES - COMMAND PROTOCOL
 * ********************************************************************************************************
*/

/* Command base opcodes (lower 4 bits) */
#define COM_CMD_EXQPI           0x00    // Exit QPI mode
#define COM_CMD_ENQPI           0x01    // Enter QPI mode
#define COM_CMD_WRBUF           0x02    // Write buffer
#define COM_CMD_WRDMA           0x03    // Write DMA
#define COM_CMD_RDBUF           0x04    // Read buffer
#define COM_CMD_RDDMA           0x05    // Read DMA
#define COM_CMD_SEG_DONE        0x06    // Segment done
#define COM_CMD_WR_DONE         0x07    // Write done
#define COM_CMD_CMD8            0x08    // Custom command 8
#define COM_CMD_CMD9            0x09    // Custom command 9
#define COM_CMD_CMDA            0x0A    // Custom command A

/* Command masks */
#define COM_CMD_MASK_BASE       0x0F    // Base command mask (lower 4 bits)
#define COM_CMD_MASK_IOMODE     0xF0    // IO mode mask (upper 4 bits)

/* IO mode flags (upper 4 bits of command byte) */
#define COM_IOMODE_1BIT         0x00    // Standard 1-bit SPI
#define COM_IOMODE_DOUT         0x10    // Dual output (data only)
#define COM_IOMODE_DIO          0x20    // Dual IO (addr + data)
#define COM_IOMODE_QOUT         0x40    // Quad output (data only)
#define COM_IOMODE_QIO          0x80    // Quad IO (addr + data)

/*
 * ********************************************************************************************************
 * TYPES
 * ********************************************************************************************************
*/

/* Communication event types (for event queue) */
typedef enum
{
    COM_EVT_RX = 0,     // Receive event
    COM_EVT_TX = 1      // Transmit event

} com_evt_t;

/* Communication mode */
typedef enum
{
    COM_MODE_SPI,
    COM_MODE_QSPI

} com_mode_t;

/* Command header structure (2 bytes: cmd + addr) */
typedef struct
{
    uint8_t cmd;        // Command byte
    uint8_t addr;       // Address byte

} com_cmd_hdr_t;

/* Communication structure */
typedef struct
{
    com_mode_t mode;

    uint8_t tx_data[COM_FRAME_SIZE];
    uint8_t rx_data[COM_FRAME_SIZE];
    size_t length;

} com_t;

/* Event frame structure for queue */
typedef struct
{
    uint8_t type;       // Event type
    uint8_t param;      // Event parameter
    uint16_t len;       // Data length
    uint8_t data[COM_FRAME_SIZE];  // Event data

} com_event_frame_t;

/*
 * ********************************************************************************************************
 * FUNCTION PROTOTYPES
 * ********************************************************************************************************
*/

void com_init(void);

/* RX Event callback (weak, can be overridden by user) */
void com_on_event_rx(uint8_t cmd, uint8_t addr, uint8_t *data, size_t len);

/* TX Event callback (weak, can be overridden by user) */
void com_on_event_tx(uint8_t cmd, uint8_t addr, uint8_t *data, size_t len);

/*
 * ********************************************************************************************************
 * END
 * ********************************************************************************************************
*/

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _COM_H_ */