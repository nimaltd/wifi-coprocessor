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

#include "wcp_qspi.h"

#include "wcp_conf.h"

#if (WCP_SELECTED_MODE != WCP_MODE_SPI_1_1_1)

#include <string.h>

#include "quadspi.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
 */

#define WCP_HD_BASE_CMD_WRDMA   0x03U
#define WCP_HD_BASE_CMD_RDDMA   0x04U
#define WCP_HD_BASE_CMD_WR_END  0x07U
#define WCP_HD_BASE_CMD_INT0    0x08U

#define WCP_HD_CMD_MOD_1_1_1    0x00U
#define WCP_HD_CMD_MOD_1_1_2    0x10U
#define WCP_HD_CMD_MOD_1_2_2    0x50U
#define WCP_HD_CMD_MOD_1_1_4    0x20U
#define WCP_HD_CMD_MOD_1_4_4    0xA0U

#define WCP_HD_ADDR_DEFAULT     0x00U
#define WCP_HD_DUMMY_CYCLES     8U

/*
 * *********************************************************************************************************
 * Private Function Prototypes
 * *********************************************************************************************************
 */

static void         wcp_qspi_apply_mode(QSPI_CommandTypeDef *cmd);
static wcp_status_t wcp_status_from_hal(HAL_StatusTypeDef hal_status);
static uint8_t      wcp_qspi_build_hd_command(uint8_t base_command);
static wcp_status_t wcp_qspi_send_hd_end_command(uint8_t instruction, uint32_t timeout_ms);

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Apply the selected QSPI mode to the given command structure.
 * @param cmd: Pointer to the QSPI command structure to be configured.
 */
static void wcp_qspi_apply_mode(QSPI_CommandTypeDef *cmd)
{
#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_1_1_1)
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_1_LINE;
    cmd->DataMode = QSPI_DATA_1_LINE;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_1_LINE;
    cmd->DataMode = QSPI_DATA_2_LINES;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_2_LINES;
    cmd->DataMode = QSPI_DATA_2_LINES;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_1_LINE;
    cmd->DataMode = QSPI_DATA_4_LINES;
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_4_LINES;
    cmd->DataMode = QSPI_DATA_4_LINES;
#else
    cmd->InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd->AddressMode = QSPI_ADDRESS_1_LINE;
    cmd->DataMode = QSPI_DATA_1_LINE;
#endif
}

/**********************************************************************************************************/
/**
 * @brief Convert HAL status code to WCP status code.
 * @param hal_status: HAL status value.
 * @return WCP status.
 */
static wcp_status_t wcp_status_from_hal(HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
    {
    case HAL_OK:
        return WCP_STATUS_OK;
    case HAL_TIMEOUT:
        return WCP_STATUS_TIMEOUT;
    default:
        return WCP_STATUS_COMM_ERROR;
    }
}

/**********************************************************************************************************/
/**
 * @brief Build SPI slave HD command with the line-mode modifier expected by ESP-IDF.
 * @param base_command: Base SPI slave HD command (0x01..0x0A range).
 * @return Encoded command byte for the selected communication mode.
 */
static uint8_t wcp_qspi_build_hd_command(uint8_t base_command)
{
#if (WCP_SELECTED_MODE == WCP_MODE_QSPI_DOUT_1_1_2)
    return (uint8_t)(base_command | WCP_HD_CMD_MOD_1_1_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_DIO_1_2_2)
    return (uint8_t)(base_command | WCP_HD_CMD_MOD_1_2_2);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QOUT_1_1_4)
    return (uint8_t)(base_command | WCP_HD_CMD_MOD_1_1_4);
#elif (WCP_SELECTED_MODE == WCP_MODE_QSPI_QIO_1_4_4)
    return (uint8_t)(base_command | WCP_HD_CMD_MOD_1_4_4);
#else
    return (uint8_t)(base_command | WCP_HD_CMD_MOD_1_1_1);
#endif
}

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
 */

/**********************************************************************************************************/
/**
 * @brief Initialize QSPI backend resources.
 */
void wcp_qspi_init(void)
{
    /* QSPI peripheral is initialized in MX_QUADSPI_Init(). */
}

/**********************************************************************************************************/
/**
 * @brief Send payload using QSPI backend.
 * @param data: Pointer to payload buffer.
 * @param len: Payload length in bytes.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_qspi_send(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef hal_status;

    if (data == 0 || len == 0U)
    {
        return WCP_STATUS_INVALID_ARG;
    }

    memset(&cmd, 0, sizeof(cmd));
    wcp_qspi_apply_mode(&cmd);
    cmd.Instruction = wcp_qspi_build_hd_command(WCP_HD_BASE_CMD_WRDMA);
    cmd.AddressSize = QSPI_ADDRESS_8_BITS;
    cmd.Address = WCP_HD_ADDR_DEFAULT;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.NbData = len;
    cmd.DummyCycles = WCP_HD_DUMMY_CYCLES;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    hal_status = HAL_QSPI_Command(&hqspi, &cmd, timeout_ms);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Transmit(&hqspi, (uint8_t *)data, timeout_ms);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    return wcp_qspi_send_hd_end_command(wcp_qspi_build_hd_command(WCP_HD_BASE_CMD_WR_END), timeout_ms);
}

/**********************************************************************************************************/
/**
 * @brief Receive payload using QSPI backend.
 * @param data: Pointer to receive buffer.
 * @param io_len: In/out length for receive operation.
 * @param timeout_ms: Timeout for transfer.
 * @return WCP status.
 */
wcp_status_t wcp_qspi_receive(uint8_t *data, uint16_t *io_len, uint32_t timeout_ms)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef hal_status;

    if (data == 0 || io_len == 0 || *io_len == 0U)
    {
        return WCP_STATUS_INVALID_ARG;
    }

    memset(&cmd, 0, sizeof(cmd));
    wcp_qspi_apply_mode(&cmd);
    cmd.Instruction = wcp_qspi_build_hd_command(WCP_HD_BASE_CMD_RDDMA);
    cmd.AddressSize = QSPI_ADDRESS_8_BITS;
    cmd.Address = WCP_HD_ADDR_DEFAULT;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.NbData = *io_len;
    cmd.DummyCycles = WCP_HD_DUMMY_CYCLES;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    hal_status = HAL_QSPI_Command(&hqspi, &cmd, timeout_ms);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    hal_status = HAL_QSPI_Receive(&hqspi, data, timeout_ms);
    if (hal_status != HAL_OK)
    {
        return wcp_status_from_hal(hal_status);
    }

    return wcp_qspi_send_hd_end_command(wcp_qspi_build_hd_command(WCP_HD_BASE_CMD_INT0), timeout_ms);
}

static wcp_status_t wcp_qspi_send_hd_end_command(uint8_t instruction, uint32_t timeout_ms)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef hal_status;

    memset(&cmd, 0, sizeof(cmd));
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction = instruction;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_8_BITS;
    cmd.Address = WCP_HD_ADDR_DEFAULT;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.NbData = 0U;
    cmd.DummyCycles = WCP_HD_DUMMY_CYCLES;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    hal_status = HAL_QSPI_Command(&hqspi, &cmd, timeout_ms);
    return wcp_status_from_hal(hal_status);
}

#endif /* (WCP_SELECTED_MODE != WCP_MODE_SPI_1_1_1) */
