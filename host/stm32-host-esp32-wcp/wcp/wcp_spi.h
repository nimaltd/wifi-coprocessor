/*
 * @file        wcp_spi.h
 * @brief       WiFi Co-Processor SPI Backend API
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

#ifndef _WCP_SPI_H_
#define _WCP_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "wcp_conf.h"

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)

#include "wcp.h"
#include "spi.h"

/*
 * *********************************************************************************************************
 * Public Function Prototypes
 * *********************************************************************************************************
 */

wcp_err_t   wcp_spi_write_data(const uint8_t *data, uint16_t len);

wcp_err_t   wcp_spi_read_data(uint8_t *data, uint16_t io_len);

wcp_err_t   wcp_spi_write_reg(uint32_t reg_addr, const uint8_t *data, uint16_t len);

wcp_err_t   wcp_spi_read_reg(uint32_t reg_addr, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1 */
#endif /* _WCP_SPI_H_ */
