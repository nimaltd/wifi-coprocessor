/*
 * @file        tpl_spi.h
 * @brief       WiFi Co-Processor SPI Transport Layer
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

#ifndef TPL_SPI_H
#define TPL_SPI_H

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * *********************************************************************************************************
 * Definitions
 * *********************************************************************************************************
*/

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
*/

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

void tpl_spi_init(void);


/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif /* TPL_SPI_H */
