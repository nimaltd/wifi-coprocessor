
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

#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/spi_slave_hd.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * *********************************************************************************************************
 * Definitions
 * *********************************************************************************************************
*/

#define TPL_FRAME_SIZE          1600
#define TPL_QUEUE_SIZE          10
#define TPL_SPI                 SPI2_HOST   
#define TPL_CMD_SIZE            8
#define TPL_REG_ADDRESS_SIZE    8
#define TPL_DUMMY_SIZE          8  
#define TPL_SPI_MODE            0 
#define TPL_TASK_STACK_SIZE     2048
#define TPL_TASK_PRIORITY       10

#define TPL_CMD_MASK            0x0F
#define TPL_CMD_WRBUF           0x01
#define TPL_CMD_RDBUF           0x02
#define TPL_CMD_WRDMA           0x03
#define TPL_CMD_RDDMA           0x04
#define TPL_CMD_SEGDONE         0x05
#define TPL_CMD_ENQPI           0x06
#define TPL_CMD_EXQPI           0xDD
#define TPL_CMD_WRDONE          0x07
#define TPL_CMD_RDDONE          0x08
#define TPL_CMD_CUSTOM1         0x09
#define TPL_CMD_CUSTOM2         0x0A

#define TPL_MOD_MASK            0xF0
#define TPL_MOD_1BIT            0x00
#define TPL_MOD_DOUT            0x10
#define TPL_MOD_DIO             0x50
#define TPL_MOD_QOUT            0x20
#define TPL_MOD_QIO             0xA0    
#define TPL_MOD_QPI             0xA0

#define TPL_REG_1               0
#define TPL_REG_2               1
#define TPL_REG_3               2       
#define TPL_REG_4               3
#define TPL_REG_5               4
#define TPL_REG_6               5
#define TPL_REG_7               6
#define TPL_REG_8               7

/*
 * *********************************************************************************************************
 * Type Definitions
 * *********************************************************************************************************
*/

typedef struct 
{
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;
    uint8_t reg5;
    uint8_t reg6;
    uint8_t reg7;
    uint8_t reg8;       
    
} tpl_regs_t;

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

void tpl_init(void);

#ifdef __cplusplus
}
#endif

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/
