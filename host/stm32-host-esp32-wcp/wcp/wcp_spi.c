/*
 * @file        wcp_spi.c
 * @brief       WiFi Co-Processor SPI Backend Implementation
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

#include "wcp_conf.h"

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)

#include "wcp_spi.h"

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Send payload using SPI backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_spi_write_data(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
    (void)WCP_TRANSFER_TIMEOUT_MS;
    return WCP_ERR_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Read payload using SPI backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: Length for receive operation.
 * @return WCP error code.
 */
wcp_err_t wcp_spi_read_data(uint8_t *data, uint16_t io_len)
{
    (void)data;
    (void)io_len;
    (void)WCP_TRANSFER_TIMEOUT_MS;
    return WCP_ERR_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Write bytes to register/buffer address using SPI backend.
 * @param reg_addr: 0~63.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_spi_write_reg(uint32_t reg_addr, const uint8_t *data, uint16_t len)
{
    (void)reg_addr;
    (void)data;
    (void)len;
    (void)WCP_TRANSFER_TIMEOUT_MS;
    return WCP_ERR_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Read bytes from register/buffer address using SPI backend.
 * @param reg_addr: 32-bit target register/buffer address.
 * @param data: Pointer to destination buffer.
 * @param len: Number of bytes to read.
 * @return WCP error code.
 */
wcp_err_t wcp_spi_read_reg(uint32_t reg_addr, uint8_t *data, uint16_t len)
{
    (void)reg_addr;
    (void)data;
    (void)len;
    (void)WCP_TRANSFER_TIMEOUT_MS;
    return WCP_ERR_NOT_IMPLEMENTED;
}

#endif /* (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1) */
