/*
 * @file        wcp_conf.h
 * @brief       WiFi Co-Processor Configuration
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

#ifndef _WCP_CONF_H
#define _WCP_CONF_H

/*
 * *********************************************************************************************************
 * Defines
 * *********************************************************************************************************
*/

/* Communication mode selection. */
#define WCP_MODE_SPI_1_1_1          0U
#define WCP_MODE_QSPI_1_1_1         1U
#define WCP_MODE_QSPI_DOUT_1_1_2    2U
#define WCP_MODE_QSPI_DIO_1_2_2     3U
#define WCP_MODE_QSPI_QOUT_1_1_4    4U
#define WCP_MODE_QSPI_QIO_1_4_4     5U

/*
 * *********************************************************************************************************
 * Configuration
 * *********************************************************************************************************
*/

/* Ready Pin */
#define WCP_READY_GPIO              GPIOB
#define WCP_READY_PIN               GPIO_PIN_2

/* Data Available Pin */
#define WCP_DATA_AVAILABLE_GPIO     GPIOB
#define WCP_DATA_AVAILABLE_PIN      GPIO_PIN_3

/* SPI/QSPI Handle */
#define WCP_HAL_HANDLE              hqspi

/* Selected communication mode. */
#define WCP_SELECTED_MODE           WCP_MODE_QSPI_QIO_1_4_4

/* Read dummy cycles for QSPI fast read transactions. */
#define WCP_QSPI_READ_DUMMY_CYCLES  8U

/* Timeout used for WCP blocking transfers (milliseconds). */
#define WCP_TRANSFER_TIMEOUT_MS     1000U

/*
 * *********************************************************************************************************
 * Check for valid configuration
 * *********************************************************************************************************
*/

#if (WCP_SELECTED_MODE > WCP_MODE_QSPI_QIO_1_4_4)
#error "Invalid WCP_SELECTED_MODE value"
#endif

#endif /* _WCP_CONF_H */
