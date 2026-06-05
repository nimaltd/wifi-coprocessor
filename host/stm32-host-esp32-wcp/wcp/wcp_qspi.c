/*
 * @file        wcp_qspi.c
 * @brief       WiFi Co-Processor QSPI Backend Implementation
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
#include <sys/_intsup.h>

#if (WCP_SELECTED_MODE != WCP_MODE_SPI_1_1_1)

#include "wcp_qspi.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

#define WCP_ESP_OPCODE_WRBUF    0x01U
#define WCP_ESP_OPCODE_RDBUF    0x02U
#define WCP_ESP_OPCODE_WRDMA    0x03U
#define WCP_ESP_OPCODE_RDDMA    0x04U
#define WCP_ESP_OPCODE_WR_END   0x07U
#define WCP_ESP_OPCODE_RD_END   0x08U

#define WCP_ESP_QSPI_MOD_1_1_1  0x00U
#define WCP_ESP_QSPI_MOD_1_1_2  0x10U
#define WCP_ESP_QSPI_MOD_1_2_2  0x50U
#define WCP_ESP_QSPI_MOD_1_1_4  0x20U
#define WCP_ESP_QSPI_MOD_1_4_4  0xA0U

#define WCP_ESP_DUMMY_CYCLES    8U

/*
 * *********************************************************************************************************
 * Private Function Prototypes
 * *********************************************************************************************************
 */

static inline wcp_err_t  wcp_status_from_hal(HAL_StatusTypeDef hal_status);
static inline wcp_err_t  wcp_qspi_send_hd_end_command(uint8_t instruction);

/*
 * *********************************************************************************************************
 * Public Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Write payload using QSPI backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_qspi_write_data(const uint8_t *data, uint16_t len)
{
    QSPI_CommandTypeDef cmd = {0};
    HAL_StatusTypeDef hal_status;

    cmd.NbData = len;
    cmd.DummyCycles = WCP_ESP_DUMMY_CYCLES;

#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_WRDMA | WCP_ESP_QSPI_MOD_1_1_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_2_LINES;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_WRDMA | WCP_ESP_QSPI_MOD_1_2_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_WRDMA | WCP_ESP_QSPI_MOD_1_1_4);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_WRDMA | WCP_ESP_QSPI_MOD_1_4_4);
#else
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.Instruction = (WCP_ESP_OPCODE_WRDMA | WCP_ESP_QSPI_MOD_1_1_1);
#endif

    hal_status = HAL_QSPI_Command(&WCP_HAL_HANDLE, &cmd, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Transmit(&WCP_HAL_HANDLE, (uint8_t *)data, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    return wcp_qspi_send_hd_end_command(WCP_ESP_OPCODE_WR_END);
}

/**********************************************************************************************************/
/**
 * @brief Read payload using QSPI backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: Length for receive operation.
 * @return WCP error code.
 */
wcp_err_t wcp_qspi_read_data(uint8_t *data, uint16_t io_len)
{
    QSPI_CommandTypeDef cmd = {0};
    HAL_StatusTypeDef hal_status;

    cmd.NbData = io_len;
    cmd.DummyCycles = WCP_ESP_DUMMY_CYCLES;

#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_RDDMA | WCP_ESP_QSPI_MOD_1_1_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_2_LINES;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_RDDMA | WCP_ESP_QSPI_MOD_1_2_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_RDDMA | WCP_ESP_QSPI_MOD_1_1_4);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = (WCP_ESP_OPCODE_RDDMA | WCP_ESP_QSPI_MOD_1_4_4);
#else
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.Instruction = (WCP_ESP_OPCODE_RDDMA | WCP_ESP_QSPI_MOD_1_1_1);
#endif

    hal_status = HAL_QSPI_Command(&WCP_HAL_HANDLE, &cmd, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Receive(&WCP_HAL_HANDLE, data, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    return wcp_qspi_send_hd_end_command(WCP_ESP_OPCODE_RD_END);
}

/**********************************************************************************************************/
/**
 * @brief Write bytes to ESP32 SPI HD slave register/buffer address.
 * @param reg_addr: 0~63.
 * @param data: pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @return WCP error code.
 */
wcp_err_t wcp_qspi_write_reg(uint32_t reg_addr, const uint8_t *data, uint16_t len)
{
    QSPI_CommandTypeDef cmd = {0};
    HAL_StatusTypeDef hal_status;

    cmd.Address = reg_addr;
    cmd.NbData = len;
    cmd.DummyCycles = WCP_ESP_DUMMY_CYCLES;

#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_WRBUF | WCP_ESP_QSPI_MOD_1_1_2;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_2_LINES;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_WRBUF | WCP_ESP_QSPI_MOD_1_2_2;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_WRBUF | WCP_ESP_QSPI_MOD_1_1_4;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_WRBUF | WCP_ESP_QSPI_MOD_1_4_4;
#else
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.Instruction = WCP_ESP_OPCODE_WRBUF | WCP_ESP_QSPI_MOD_1_1_1;
#endif

    hal_status = HAL_QSPI_Command(&WCP_HAL_HANDLE, &cmd, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Transmit(&WCP_HAL_HANDLE, (uint8_t *)data, WCP_TRANSFER_TIMEOUT_MS);
    return wcp_status_from_hal(hal_status);
}

/**********************************************************************************************************/
/**
 * @brief Read bytes from ESP32 SPI HD slave register/buffer address.
 * @param reg_addr: 0~63.
 * @param data: Pointer to destination buffer.
 * @param len: Number of bytes to read.
 * @return WCP error code.
 */
wcp_err_t wcp_qspi_read_reg(uint32_t reg_addr, uint8_t *data, uint16_t len)
{
    QSPI_CommandTypeDef cmd = {0};
    HAL_StatusTypeDef hal_status;

    cmd.Address = reg_addr;
    cmd.NbData = len;
    cmd.DummyCycles = WCP_ESP_DUMMY_CYCLES;

#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_RDBUF | WCP_ESP_QSPI_MOD_1_1_2;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_2_LINES;
    cmd.DataMode = QSPI_DATA_2_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_RDBUF | WCP_ESP_QSPI_MOD_1_2_2;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_RDBUF | WCP_ESP_QSPI_MOD_1_1_4;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.Instruction = WCP_ESP_OPCODE_RDBUF | WCP_ESP_QSPI_MOD_1_4_4;
#else
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.Instruction = WCP_ESP_OPCODE_RDBUF | WCP_ESP_QSPI_MOD_1_1_1;
#endif

    hal_status = HAL_QSPI_Command(&WCP_HAL_HANDLE, &cmd, WCP_TRANSFER_TIMEOUT_MS);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Receive(&WCP_HAL_HANDLE, data, WCP_TRANSFER_TIMEOUT_MS);
    return wcp_status_from_hal(hal_status);
}

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Convert HAL status code to WCP status code.
 * @param hal_status: HAL status value.
 * @return WCP status.
 */
static inline wcp_err_t wcp_status_from_hal(HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
    {
    case HAL_OK:
        return WCP_ERR_NONE;
    case HAL_TIMEOUT:
        return WCP_ERR_TIMEOUT;
    default:
        return WCP_ERR_UNKNOWN;
    }
}

/**********************************************************************************************************/
/**
 * @brief Send the SPI slave HD command to indicate end of WRDMA/RDDMA operation.
 * @param instruction: Base command (WR_END or RD_END) to be encoded with line-mode modifier.
 * @return WCP error code.
 */
static inline wcp_err_t wcp_qspi_send_hd_end_command(uint8_t instruction)
{
    QSPI_CommandTypeDef cmd = {0};
    HAL_StatusTypeDef hal_status;

    cmd.Instruction = instruction;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = WCP_ESP_DUMMY_CYCLES;

    hal_status = HAL_QSPI_Command(&WCP_HAL_HANDLE, &cmd, WCP_TRANSFER_TIMEOUT_MS);
    return wcp_status_from_hal(hal_status);
}

#endif /* (WCP_SELECTED_MODE != WCP_MODE_SPI_1_1_1) */
