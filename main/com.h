
/*
 * @file        com.h
 * @brief       communication module for the ESP32 HOST project.
 * @author      Nima Askari
 * @version     1.0.0
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

#ifndef _COM_H_
#define _COM_H_

#ifdef __cplusplus
extern "C" {    
#endif

/*
 * ********************************************************************************************************
 * INCLUDES
 * ********************************************************************************************************
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/spi_slave.h"  

/*
 * ********************************************************************************************************
 * DEFINES
 * ********************************************************************************************************
*/

#define COM_FRAME_SIZE          2048
#define COM_FRAME_QUEUE_SIZE    10
#define COM_TASK_STACK_SIZE     4096
#define COM_TASK_PRIORITY       10

/*
 * ********************************************************************************************************
 * TYPES
 * ********************************************************************************************************
*/

/* Communication mode */
typedef enum
{
    COM_MODE_SPI,
    COM_MODE_QSPI

} com_mode_t;

/* Communication structure */
typedef struct
{
    com_mode_t mode;

    uint8_t tx_data[COM_FRAME_SIZE];
    uint8_t rx_data[COM_FRAME_SIZE];
    size_t length;

} com_t;

/*
 * ********************************************************************************************************
 * FUNCTION PROTOTYPES
 * ********************************************************************************************************
*/

void com_init(void);

/*
 * ********************************************************************************************************
 * END
 * ********************************************************************************************************
*/

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _COM_H_ */