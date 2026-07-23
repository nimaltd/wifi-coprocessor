/*
 * @file        qspi.h
 * @brief       WiFi Co-Processor QSPI Transport Layer
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

#ifndef _QSPI_H_
#define _QSPI_H_

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct __attribute__((packed)) 
{
    /* Constant values */
    uint8_t             chip_id;
    uint8_t             ver_major;
    uint8_t             ver_minor;
    uint8_t             ver_patch; 
    
    /* Variable values */
    uint8_t             ap_status;
    uint8_t             sta_status;

} qspi_reg_read_t;

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

void qspi_init(void);

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

#ifdef __cplusplus
}
#endif

#endif /* _QSPI_H */
