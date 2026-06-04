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

#include "wcp_spi.h"

#include "wcp_conf.h"

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Initialize SPI backend resources.
 */
void wcp_spi_init(void)
{
    /* TODO: SPI backend initialization will be added later. */
}

/**********************************************************************************************************/
/**
 * @brief Send payload using SPI backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_spi_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    (void)data;
    (void)len;
    (void)timeout_ms;
    return WCP_STATUS_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Receive payload using SPI backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: In/out length for receive operation.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_spi_receive(uint8_t *data, uint16_t *io_len, uint32_t timeout_ms)
{
    (void)data;
    (void)io_len;
    (void)timeout_ms;
    return WCP_STATUS_NOT_IMPLEMENTED;
}

#endif /* (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1) */
