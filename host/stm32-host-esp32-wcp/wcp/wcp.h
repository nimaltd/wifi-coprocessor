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
    WCP_ERR_NONE = 0,
    WCP_ERR_INVALID_ARG,
    WCP_ERR_TIMEOUT,
    WCP_ERR_NOT_IMPLEMENTED,
    WCP_ERR_UNKNOWN

} wcp_err_t;

/*
 * *********************************************************************************************************
 * Public Function Prototypes
 * *********************************************************************************************************
 */

void        wcp_init(void);

wcp_err_t   wcp_write_data(const uint8_t *data, uint16_t len);

wcp_err_t   wcp_read_data(uint8_t *data, uint16_t io_len);

wcp_err_t   wcp_write_reg(uint32_t reg_addr, const uint8_t *data, uint16_t len);

wcp_err_t   wcp_read_reg(uint32_t reg_addr, uint8_t *data, uint16_t len);

wcp_err_t   wcp_network_init(void);

wcp_err_t   wcp_wifi_control(uint32_t command, const uint8_t *payload, uint16_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /* WCP_WCP_H */
