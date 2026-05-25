/*
 * @file        tpl_usart.h
 * @brief       WiFi Co-Processor USART Transport Layer
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

#ifndef TPL_USART_H
#define TPL_USART_H

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/uart.h"
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

void tpl_usart_init(void);


/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif /* TPL_USART_H */
