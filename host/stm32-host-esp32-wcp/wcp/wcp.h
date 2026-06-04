/*
 * @file        wcp.h
 * @brief       WiFi Co-Processor Host API
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
 * Copyright (C) 2025 Nima Askari - NimaLTD. All rights reserved.
*/

#ifndef _WCP_WCP_H_
#define _WCP_WCP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
 */

typedef enum
{
    WCP_STATUS_OK = 0,
    WCP_STATUS_INVALID_ARG,
    WCP_STATUS_COMM_ERROR,
    WCP_STATUS_TIMEOUT,
    WCP_STATUS_NOT_IMPLEMENTED
    
} wcp_status_t;

/*
 * *********************************************************************************************************
 * Public Function Prototypes
 * *********************************************************************************************************
 */

void wcp_init(void);

wcp_status_t wcp_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

wcp_status_t wcp_receive(uint8_t *data, uint16_t *io_len, uint32_t timeout_ms);

wcp_status_t wcp_network_init(void);

wcp_status_t wcp_wifi_control(uint32_t command, const uint8_t *payload, uint16_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /* WCP_WCP_H */
