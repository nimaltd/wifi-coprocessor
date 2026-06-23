/*
 * @file        types.h
 * @brief       WiFi Co-Processor Types
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

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
*/

enum
{
    TYP_STA_DISABLED            = 0x00,
    TYP_STA_CONNECTING          = 0x01,
    TYP_STA_CONNECTED           = 0x02,
    TYP_STA_DISCONNECTED        = 0x03,
    TYP_STA_ERROR               = 0xFF,
};

enum
{
    TYP_AP_DISABLED             = 0x00,
    TYP_AP_CLIENT_CNT0          = 0x10,
    TYP_AP_CLIENT_CNT1          = 0x11,
    TYP_AP_CLIENT_CNT2          = 0x12,
    TYP_AP_CLIENT_CNT3          = 0x13,
    TYP_AP_CLIENT_CNT4          = 0x14,
    TYP_AP_CLIENT_CNT5          = 0x15,
    TYP_AP_CLIENT_CNT6          = 0x16,
    TYP_AP_CLIENT_CNT7          = 0x17,
    TYP_AP_CLIENT_CNT8          = 0x18,
    TYP_AP_CLIENT_CNT9          = 0x19,
    TYP_AP_CLIENT_CNT10         = 0x1A,
    TYP_AP_CLIENT_CNT11         = 0x1B,
    TYP_AP_CLIENT_CNT12         = 0x1C,
    TYP_AP_CLIENT_CNT13         = 0x1D,
    TYP_AP_CLIENT_CNT14         = 0x1E,
    TYP_AP_CLIENT_CNT15         = 0x1F,
    TYP_AP_CLIENT_CNT16         = 0x20,
    TYP_AP_ERROR                = 0xFF,
};

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#endif /* _TYPES_H_ */
