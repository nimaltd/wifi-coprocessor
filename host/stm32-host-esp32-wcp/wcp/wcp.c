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

}

/**********************************************************************************************************/
/**
 * @brief Send payload through selected WCP communication backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_write_data(const uint8_t *data, uint16_t len)
{
    if (data == 0 || len == 0U)
    {
        return WCP_ERR_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_write_data(data, len);
#else
    return wcp_qspi_write_data(data, len);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Receive payload through selected WCP communication backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: Length for receive operation.
 * @return WCP error code.
 */
wcp_err_t wcp_read_data(uint8_t *data, uint16_t io_len)
{
    if (data == 0 || io_len == 0U)
    {
        return WCP_ERR_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_read_data(data, io_len);
#else
    return wcp_qspi_read_data(data, io_len);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Write bytes to ESP32 SPI HD slave register/buffer address.
 * @param reg_addr: 0~63.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_write_reg(uint32_t reg_addr, const uint8_t *data, uint16_t len)
{
    if (data == 0 || len == 0U || reg_addr > 63U)
    {
        return WCP_ERR_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_write_reg(reg_addr, data, len);
#else
    return wcp_qspi_write_reg(reg_addr, data, len);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Read bytes from ESP32 SPI HD slave register/buffer address.
 * @param reg_addr: 0~63.
 * @param data: Pointer to destination buffer.
 * @param len: Number of bytes to read.
 * @return WCP error code.
 */
wcp_err_t wcp_read_reg(uint32_t reg_addr, uint8_t *data, uint16_t len)
{
    if (data == 0 || len == 0U || reg_addr > 63U)
    {
        return WCP_ERR_INVALID_ARG;
    }

#if (WCP_SELECTED_MODE == WCP_MODE_SPI_1_1_1)
    return wcp_spi_read_reg(reg_addr, data, len);
#else
    return wcp_qspi_read_reg(reg_addr, data, len);
#endif
}

/**********************************************************************************************************/
/**
 * @brief Initialize future network control layer.
 * @return WCP error code.
 */
wcp_err_t wcp_network_init(void)
{
    return WCP_ERR_NOT_IMPLEMENTED;
}

/**********************************************************************************************************/
/**
 * @brief Handle future WiFi control command path.
 * @param command: Control command identifier.
 * @param payload: Optional command payload.
 * @param payload_len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_wifi_control(uint32_t command, const uint8_t *payload, uint16_t payload_len)
{
    (void)command;
    (void)payload;
    (void)payload_len;
    return WCP_ERR_NOT_IMPLEMENTED;
}
