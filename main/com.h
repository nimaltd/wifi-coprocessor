
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

#include <stdio.h>
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"
#include "driver/gpio.h"
#include <driver/uart.h>
#include "sdkconfig.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {    
#endif

void com_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _COM_H_ */