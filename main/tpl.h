
/*
 * @file        tpl.h
 * @brief       WiFi Co-Processor Transport Layer
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

#ifndef TPL_H
#define TPL_H

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * *********************************************************************************************************
 * Definitions
 * *********************************************************************************************************
*/

#define TPL_MAX_PAYLOAD_SIZE    1500
#define TPL_TASK_STACK_SIZE     4096
#define TPL_TASK_PRIORITY       5

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
*/

/* List of commands */
typedef enum
{
    TPL_CMD_VERSION             = 0,

} tpl_cmd_t;

/* List of events */
typedef enum
{
    TPL_EVT_STA_CONNECTED       = 0,
    TPL_EVT_STA_DISCONNECTED,
    TPL_EVT_AP_CONNECTED,
    TPL_EVT_AP_DISCONNECTED,
    TPL_EVT_ERROR,

} tpl_evt_t;

/* List of results */
typedef enum
{
    TPL_RES_OK                  = 0,
    TPL_RES_ERROR,

} tpl_status_t;

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

void tpl_init(void);

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif /* TPL_H */
