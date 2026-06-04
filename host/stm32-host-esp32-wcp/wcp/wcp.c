/*
 * @file        wcp.c
 * @brief       WiFi Co-Processor Host API Implementation
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

#include "wcp.h"

#include "wcp_conf.h"

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
#include "wcp_spi.h"
#else
#include "wcp_qspi.h"
#endif

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Initialize the WCP communication backend based on selected mode.
 */
void wcp_init(void)
{
#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    wcp_spi_init();
#else
    wcp_qspi_init();
#endif
}

/**********************************************************************************************************/
/**
 * @brief Send payload through selected WCP communication backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (data == 0 || len == 0U)
    {
        return WCP_STATUS_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_send(data, len, timeout_ms);
#else
    return wcp_qspi_send(data, len, timeout_ms);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Receive payload through selected WCP communication backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: In/out length for receive operation.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_receive(uint8_t *data, uint16_t *io_len, uint32_t timeout_ms)
{
    if (data == 0 || io_len == 0 || *io_len == 0U)
    {
        return WCP_STATUS_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_receive(data, io_len, timeout_ms);
#else
    return wcp_qspi_receive(data, io_len, timeout_ms);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Initialize future network control layer.
 * @return WCP status.
 */
wcp_status_t wcp_network_init(void)
{
    return WCP_STATUS_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Handle future WiFi control command path.
 * @param command: Control command identifier.
 * @param payload: Optional command payload.
 * @param payload_len: Payload length in bytes.
 * @return WCP status.
 */
wcp_status_t wcp_wifi_control(uint32_t command, const uint8_t *payload, uint16_t payload_len)
{
    (void)command;
    (void)payload;
    (void)payload_len;
    return WCP_STATUS_NOT_IMPLEMENTED;
}
