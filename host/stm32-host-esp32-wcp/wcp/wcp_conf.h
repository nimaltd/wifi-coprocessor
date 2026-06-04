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
 * Communication Mode Selection
 * *********************************************************************************************************
 */

/* Communication mode selection. */
#define WCP_MODE_SPI_1_1_1       0U
#define WCP_MODE_QSPI_1_1_1      1U
#define WCP_MODE_QSPI_DOUT_1_1_2 2U
#define WCP_MODE_QSPI_DIO_1_2_2  3U
#define WCP_MODE_QSPI_QOUT_1_1_4 4U
#define WCP_MODE_QSPI_QIO_1_4_4  5U

/*
 * Select active communication mode:
 * - WCP_MODE_SPI_1_1_1
 * - WCP_MODE_QSPI_1_1_1
 * - WCP_MODE_QSPI_DOUT_1_1_2
 * - WCP_MODE_QSPI_DIO_1_2_2
 * - WCP_MODE_QSPI_QOUT_1_1_4
 * - WCP_MODE_QSPI_QIO_1_4_4
 */
#define WCP_SELECTED_MODE WCP_MODE_QSPI_QIO_1_4_4

/* Read dummy cycles for QSPI fast read transactions. */
#define WCP_QSPI_READ_DUMMY_CYCLES 8U

#if (WCP_SELECTED_MODE > WCP_MODE_QSPI_QIO_1_4_4)
#error "Invalid WCP_SELECTED_MODE value"
#endif

#endif /* _WCP_CONF_H */
