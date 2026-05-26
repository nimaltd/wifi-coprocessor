
/*
 * @file        tpl.h
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

#ifndef TPL_H
#define TPL_H

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_rom_crc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * *********************************************************************************************************
 * Definitions
 * *********************************************************************************************************
*/

#define TPL_MAX_PAYLOAD_SIZE    1500
#define TPL_TASK_STACK_SIZE     4096
#define TPL_TASK_PRIORITY       5
#define TPL_CRC_POLY            0x1021

#define TPL_CMD_READ_MASK       0x80 

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
*/

/* List of interfaces */
typedef enum
{
    TPL_INTERFACE_NONE          = 0x00,
    TPL_INTERFACE_SPI           = 0x01,
    TPL_INTERFACE_USART         = 0x02,

} tpl_interface_t;

/* List of commands */
typedef enum
{
    TPL_CMD_FACTORY_RESET       = 0x00,
    TPL_CMD_VERSION             = 0x01,
    TPL_CMD_RESTART             = 0x02,
    TPL_CMD_RESULT              = 0x03,    

} tpl_cmd_t;

/* List of events */
typedef enum
{
    TPL_EVT_STA_CONNECTED       = 0x00,
    TPL_EVT_STA_DISCONNECTED,
    TPL_EVT_AP_CONNECTED        = 0x20,
    TPL_EVT_AP_DISCONNECTED,
    TPL_EVT_ERROR,

} tpl_evt_t;

/* List of results */
typedef enum
{
    TPL_RES_OK                  = 0x00,
    TPL_RES_ERR_CMD,
    TPL_RES_ERR_PARAM,
    TPL_RES_ERR_GENERAL,

} tpl_result_t;

typedef struct __attribute__((packed))
{
    uint8_t                     cmd;                            /* Command code */
    uint16_t                    payload_len;                    /* Length of the payload */
    uint8_t                     payload[TPL_MAX_PAYLOAD_SIZE];  /* Payload data */    
    uint16_t                    crc;                            /* CRC16 of the payload */

} tpl_rx_packet_t;

typedef struct __attribute__((packed))
{
    uint8_t                     result;                         /* Result */
    uint16_t                    payload_len;                    /* Length of the payload */
    uint8_t                     payload[TPL_MAX_PAYLOAD_SIZE];  /* Payload data */
    uint16_t                    crc;                            /* CRC16 of the payload */

} tpl_tx_packet_t;

typedef struct __attribute__((packed))
{
    uint8_t                     result;                         /* Result */
    uint16_t                    payload_len;                    /* Length of the payload */
    uint8_t                     payload[TPL_MAX_PAYLOAD_SIZE];  /* Payload data */

} tpl_res_packet_t;

typedef struct
{
    tpl_interface_t             interface;      /* Interface on which the last command was received (for response) */    
    tpl_rx_packet_t             rx_packet;      /* Received packet from host, filled by SPI/USART task */    
    tpl_tx_packet_t             tx_packet;      /* Response for read commands, sent back immediately on read */
    tpl_res_packet_t            wr_response;    /* Response for write commands, sent back on next read after write command is processed */

} tpl_t;

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

void tpl_init(void);

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif /* TPL_H */
