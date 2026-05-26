/*
 * @file        tpl.c
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "tpl.h"
#include "tpl_spi.h"
#include "tpl_usart.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

#define TAG "TPL"

/*
 * *********************************************************************************************************
 * Global Variables
 * *********************************************************************************************************
*/

tpl_t   tpl = {0};

/*
 * *********************************************************************************************************
 * Private Function Prototypes
 * *********************************************************************************************************
*/

void tpl_task(void *parameters);
void tpl_cmd_rd_check(tpl_cmd_t cmd);
void tpl_cmd_wr_check(tpl_cmd_t cmd, uint8_t *payload, uint16_t payload_len);
void tpl_response(tpl_result_t result, uint8_t *payload, uint16_t payload_len);                                                                           
void tpl_wr_response(tpl_result_t result, uint8_t *payload, uint16_t payload_len);

void tpl_restart_timer_cb(TimerHandle_t xTimer);

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
*/

void tpl_init(void)
{
    /* LED pin (output, idle off) */
    gpio_config_t io =
    {
        .pin_bit_mask = (1ULL << CONFIG_LED_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    gpio_set_level(CONFIG_LED_PIN, !CONFIG_LED_ACTIVE_LEVEL);

    /* Event pin (output, idle low) */
    io.pin_bit_mask = (1ULL << CONFIG_EVENT_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_EVENT_PIN, 0);

    /* READY pin (output, idle low) */
    io.pin_bit_mask = (1ULL << CONFIG_READY_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_READY_PIN, 0);

    tpl_spi_init();
    tpl_usart_init();

    xTaskCreate(tpl_task, "tpl_task", TPL_TASK_STACK_SIZE, NULL, TPL_TASK_PRIORITY, NULL);
}

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief Main task for the Transport Layer. This task will handle all the communication with the host.
 *        It will read commands from the host, execute them, and send back responses.
 */
void tpl_task(void *parameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**********************************************************************************************************/
/**
 * @brief Check and handle read commands.
 * @param cmd The command to check.
 */
void tpl_cmd_rd_check(tpl_cmd_t cmd)
{
    bool is_read_cmd = (cmd & TPL_CMD_READ_MASK) != 0;

    if (is_read_cmd)
    {
        /* This is a read command, handle it accordingly */
        switch (cmd)
        {
            /********************************************/
            /* TPL READ: VERSION                        */
            /********************************************/
            case TPL_CMD_VERSION:
                {
                    ESP_LOGI(TAG, "CMD: VERSION");
                    tpl_response(TPL_RES_OK, (uint8_t *)PROJECT_VER, strlen(PROJECT_VER));
                }
                break;

            /********************************************/
            /* TPL READ: RESULT                         */
            /********************************************/
            case TPL_CMD_RESULT:
                {
                    ESP_LOGI(TAG, "CMD: RESULT");
                    tpl_response(tpl.wr_response.result, tpl.wr_response.payload, tpl.wr_response.payload_len);
                }
                break;    

            /********************************************/
            /* TPL READ: UNKNOWN                        */
            /********************************************/
            default:
                {
                    ESP_LOGW(TAG, "CMD RD: Unknown [0x%02X]", cmd);
                    tpl_response(TPL_RES_ERR_CMD, NULL, 0);
                }
                break;
        }
    }
}

/**********************************************************************************************************/
/**
 * @brief Check and handle write commands.
 * @param cmd The command to check.
 * @param payload The payload of the command.
 * @param payload_len The length of the payload.
 */
void tpl_cmd_wr_check(tpl_cmd_t cmd, uint8_t *payload, uint16_t payload_len)
{
    bool is_read_cmd = (cmd & TPL_CMD_READ_MASK) != 0;

    if (!is_read_cmd)
    {
        /* This is a write command, handle it accordingly */
        switch (cmd)
        {
            /********************************************/
            /* TPL WRITE: FACTORY RESET                 */
            /********************************************/
            case TPL_CMD_FACTORY_RESET:
                {
                    ///TODO: Implement factory reset functionality
                    ESP_LOGI(TAG, "CMD: FACTORY RESET");
                    tpl_wr_response(TPL_RES_OK, NULL, 0);
                }
                break;

            /********************************************/
            /* TPL WRITE: RESTART                       */
            /********************************************/
            case TPL_CMD_RESTART:
                {
                    ESP_LOGI(TAG, "CMD: RESTART");
                    TimerHandle_t restart_timer = xTimerCreate("restart_timer", pdMS_TO_TICKS(100), pdFALSE, NULL, tpl_restart_timer_cb);
                    xTimerStart(restart_timer, 0);
                    if (restart_timer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to create restart timer");
                        tpl_wr_response(TPL_RES_ERR_GENERAL, NULL, 0);
                    }
                    else
                    {
                        tpl_wr_response(TPL_RES_OK, NULL, 0);
                    }
                }
                break;

            /********************************************/
            /* TPL WRITE: UNKNOWN                       */
            /********************************************/                
            default:
                {
                    ESP_LOGW(TAG, "CMD WR: Unknown [0x%02X]", cmd);
                    tpl_wr_response(TPL_RES_ERR_CMD, NULL, 0);
                }
                break;
        }
    }
}

/**********************************************************************************************************/
/** 
 * @brief Send a response back to the host.
 * @param result The result of the response.
 * @param payload The payload to send back (can be NULL if no payload).
 * @param payload_len The length of the payload (0 if no payload).
 */
void tpl_response(tpl_result_t result, uint8_t *payload, uint16_t payload_len)
{
    tpl.tx_packet.result = result;
    
    if (payload != NULL && payload_len > 0 && payload_len <= TPL_MAX_PAYLOAD_SIZE)
    {
        memcpy(tpl.tx_packet.payload, payload, payload_len);
        tpl.tx_packet.payload_len = payload_len;
    }
    else
    {
        tpl.tx_packet.payload_len = 0;
    }

    tpl.tx_packet.crc = esp_rom_crc16_le(TPL_CRC_POLY, (uint8_t*)&tpl.tx_packet, tpl.tx_packet.payload_len + 3);
}

/**********************************************************************************************************/
/**
 * @brief Set the result for a write command.
 * @param result The result of the write command.
 * @param payload The payload to send back (can be NULL if no payload).
 * @param payload_len The length of the payload (0 if no payload).
 */
void tpl_wr_response(tpl_result_t result, uint8_t *payload, uint16_t payload_len)
{
    tpl.wr_response.result = result;

    if (payload != NULL && payload_len > 0 && payload_len <= TPL_MAX_PAYLOAD_SIZE)
    {
        memcpy(tpl.wr_response.payload, payload, payload_len);
        tpl.wr_response.payload_len = payload_len;
    }
    else
    {
        tpl.wr_response.payload_len = 0;
    }
}

/**********************************************************************************************************/
/**
 * @brief Timer callback function for restarting the device.
 * @param xTimer The timer handle (not used).
 */
void tpl_restart_timer_cb(TimerHandle_t xTimer)
{
    esp_restart();
}

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/

