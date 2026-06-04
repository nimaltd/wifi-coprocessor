#ifndef _WCP_QSPI_H_
#define _WCP_QSPI_H_

/*
 * @file        wcp_qspi.h
 * @brief       WiFi Co-Processor QSPI Backend API
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

#ifdef __cplusplus
extern "C" {
#endif

#include "wcp.h"

/*
 * *********************************************************************************************************
 * Public Function Prototypes
 * *********************************************************************************************************
 */

void            wcp_qspi_init(void);

wcp_status_t    wcp_qspi_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

wcp_status_t    wcp_qspi_receive(uint8_t *data, uint16_t *io_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* _WCP_QSPI_H_ */
