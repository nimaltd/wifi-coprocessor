/*
 * @file        qspi.c
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

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_rom_crc.h"
#include "driver/gpio.h"
#include "driver/spi_slave_hd.h"
#include "qspi.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

#define TAG                         "QSPI"

#define QSPI_HOST                   SPI2_HOST
#define QSPI_QUEUE_SIZE             4U
#define QSPI_RX_BUF_LEN             1536U
#define QSPI_TX_BUF_LEN             1536U

#define QSPI_REG_READY              0
#define QSPI_REG_RX_COUNT           4
#define QSPI_REG_TX_COUNT           8
#define QSPI_REG_LAST_RX_LEN        12
#define QSPI_REG_LAST_RX_CRC        16
#define QSPI_REG_TEST_PASS          20

#define QSPI_READY_FLAG             0x51535049UL /* 'QSPI' */
#define QSPI_TEST_DATA_LEN          64U
#define QSPI_TX_PREFIX              "ECHO:"

/*
 * *********************************************************************************************************
 * Types
 * *********************************************************************************************************
*/

typedef struct
{
    spi_slave_hd_data_t     trans;
    uint8_t                 *buf;

} qspi_desc_t;

typedef struct
{
    uint32_t                rx_count;
    uint32_t                tx_count;
    uint32_t                last_rx_len;
    uint32_t                last_rx_crc;
    uint32_t                test_pass;

} qspi_stats_t;

typedef struct
{
    size_t                  len;
    uint8_t                 data[QSPI_TX_BUF_LEN];

} qspi_tx_msg_t;

/*
 * *********************************************************************************************************
 * Global Variables
 * *********************************************************************************************************
*/

static TaskHandle_t         qspi_rx_task_handle = NULL;
static TaskHandle_t         qspi_tx_task_handle = NULL;
static qspi_desc_t          rx_desc[QSPI_QUEUE_SIZE];
static qspi_desc_t          tx_desc[QSPI_QUEUE_SIZE];
static qspi_stats_t         qspi_stats = {0};
static QueueHandle_t        qspi_tx_queue = NULL;
static SemaphoreHandle_t    qspi_stats_mutex = NULL;

/*
 * *********************************************************************************************************
 * Private Function Prototypes
 * *********************************************************************************************************
*/

static void     qspi_rx_task(void *parameters);
static void     qspi_tx_task(void *parameters);
static void     qspi_init_gpio(void);
static void     qspi_init_slave_hd(void);
static void     qspi_init_descriptors(void);
static void     qspi_update_shared_regs(void);
static uint16_t qspi_crc16(const uint8_t *data, size_t len);
static size_t   qspi_align4(size_t value);
static size_t   qspi_prepare_default_tx(uint8_t *out, size_t out_len);
static size_t   qspi_build_rx_response(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len);

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
*/

void qspi_init(void)
{
    qspi_stats_mutex = xSemaphoreCreateMutex();
    assert(qspi_stats_mutex != NULL);

    qspi_tx_queue = xQueueCreate(1, sizeof(qspi_tx_msg_t));
    assert(qspi_tx_queue != NULL);

    qspi_init_gpio();
    qspi_init_slave_hd();
    qspi_init_descriptors();
    qspi_update_shared_regs();

    xTaskCreate(qspi_rx_task, "qspi_rx_task", 4096, NULL, 5, &qspi_rx_task_handle);
    xTaskCreate(qspi_tx_task, "qspi_tx_task", 4096, NULL, 5, &qspi_tx_task_handle);
    ESP_LOGI(TAG, "QSPI initialized (ESP-IDF v6 slave HD, segment mode)");
}

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief Initialize GPIOs for QSPI communication and status indication.
 */
static void qspi_init_gpio(void)
{
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

    io.pin_bit_mask = (1ULL << CONFIG_EVENT_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_EVENT_PIN, 0);

    io.pin_bit_mask = (1ULL << CONFIG_READY_PIN);
    gpio_config(&io);
    gpio_set_level(CONFIG_READY_PIN, 1);
}

/**********************************************************************************************************/
/**
 * @brief Initialize the SPI slave HD interface for QSPI communication.
 */
static void qspi_init_slave_hd(void)
{
    spi_bus_config_t bus_cfg =
    {
        .sclk_io_num     = CONFIG_SPI_SCK_PIN,
        .mosi_io_num     = CONFIG_SPI_D0_PIN,
        .miso_io_num     = CONFIG_SPI_D1_PIN,
        .quadwp_io_num   = CONFIG_SPI_D2_PIN,
        .quadhd_io_num   = CONFIG_SPI_D3_PIN,
        .max_transfer_sz = QSPI_RX_BUF_LEN,
        .flags           = 0,
        .intr_flags      = 0,
    };

    spi_slave_hd_slot_config_t slot_cfg =
    {
        .mode         = 0,
        .spics_io_num = CONFIG_SPI_CS_PIN,
        .flags        = 0,
        .command_bits = 8,
        .address_bits = 8,
        .dummy_bits   = 8,
        .queue_size   = QSPI_QUEUE_SIZE,
        .dma_chan     = SPI_DMA_CH_AUTO,
        .cb_config    = (spi_slave_hd_callback_config_t){0},
    };

    ESP_ERROR_CHECK(spi_slave_hd_init(QSPI_HOST, &bus_cfg, &slot_cfg));
}

/**********************************************************************************************************/
/**
 * @brief Initialize the descriptors for QSPI communication.
 */
static void qspi_init_descriptors(void)
{
    for (uint32_t i = 0; i < QSPI_QUEUE_SIZE; i++)
    {
        rx_desc[i].buf = heap_caps_calloc(1, QSPI_RX_BUF_LEN, MALLOC_CAP_DMA);
        tx_desc[i].buf = heap_caps_calloc(1, QSPI_TX_BUF_LEN, MALLOC_CAP_DMA);
        assert(rx_desc[i].buf != NULL);
        assert(tx_desc[i].buf != NULL);

        rx_desc[i].trans = (spi_slave_hd_data_t)
        {
            .data  = rx_desc[i].buf,
            .len   = QSPI_RX_BUF_LEN,
            .flags = SPI_SLAVE_HD_TRANS_DMA_BUFFER_ALIGN_AUTO,
            .arg   = (void *)(uintptr_t)i,
        };

        tx_desc[i].trans = (spi_slave_hd_data_t)
        {
            .data  = tx_desc[i].buf,
            .len   = qspi_align4(qspi_prepare_default_tx(tx_desc[i].buf, QSPI_TX_BUF_LEN)),
            .flags = SPI_SLAVE_HD_TRANS_DMA_BUFFER_ALIGN_AUTO,
            .arg   = (void *)(uintptr_t)i,
        };
    }
}

/**********************************************************************************************************/
/**
 * @brief Task to handle incoming QSPI transactions from the master.
 *        It processes received data, updates statistics, and prepares responses.
 * @param parameters Unused task parameters.
 */
static void qspi_rx_task(void *parameters)
{
    (void)parameters;

    for (uint32_t i = 0; i < QSPI_QUEUE_SIZE; i++)
    {
        ESP_ERROR_CHECK(spi_slave_hd_queue_trans(QSPI_HOST, SPI_SLAVE_CHAN_RX, &rx_desc[i].trans, portMAX_DELAY));
    }

    while (1)
    {
        spi_slave_hd_data_t *rx_done = NULL;
        ESP_ERROR_CHECK(spi_slave_hd_get_trans_res(QSPI_HOST, SPI_SLAVE_CHAN_RX, &rx_done, portMAX_DELAY));

        size_t rx_len = rx_done->trans_len;
        qspi_tx_msg_t msg = {0};
        msg.len = qspi_build_rx_response(rx_done->data, rx_len, msg.data, sizeof(msg.data));

        xSemaphoreTake(qspi_stats_mutex, portMAX_DELAY);
        qspi_stats.rx_count++;
        qspi_stats.last_rx_len = (uint32_t)rx_len;
        qspi_stats.last_rx_crc = qspi_crc16(rx_done->data, rx_len);
        xSemaphoreGive(qspi_stats_mutex);

        xQueueOverwrite(qspi_tx_queue, &msg);
        gpio_set_level(CONFIG_EVENT_PIN, 1);
        qspi_update_shared_regs();
        ESP_ERROR_CHECK(spi_slave_hd_queue_trans(QSPI_HOST, SPI_SLAVE_CHAN_RX, rx_done, portMAX_DELAY));
    }
}

/**********************************************************************************************************/
/**
 * @brief Task to handle outgoing QSPI transactions to the master.
 *        It prepares data to be sent, updates statistics, and manages the transmission queue.
 * @param parameters Unused task parameters.
 */
static void qspi_tx_task(void *parameters)
{
    (void)parameters;

    for (uint32_t i = 0; i < QSPI_QUEUE_SIZE; i++)
    {
        ESP_ERROR_CHECK(spi_slave_hd_queue_trans(QSPI_HOST, SPI_SLAVE_CHAN_TX, &tx_desc[i].trans, portMAX_DELAY));
    }

    while (1)
    {
        spi_slave_hd_data_t *tx_done = NULL;
        ESP_ERROR_CHECK(spi_slave_hd_get_trans_res(QSPI_HOST, SPI_SLAVE_CHAN_TX, &tx_done, portMAX_DELAY));

        qspi_tx_msg_t msg = {0};
        if (xQueueReceive(qspi_tx_queue, &msg, 0) == pdTRUE)
        {
            size_t tx_len = qspi_align4(msg.len);
            if (tx_len > QSPI_TX_BUF_LEN)
            {
                tx_len = QSPI_TX_BUF_LEN;
            }

            memset(tx_done->data, 0, tx_len);
            memcpy(tx_done->data, msg.data, msg.len);
            tx_done->len = tx_len;
            gpio_set_level(CONFIG_EVENT_PIN, 0);
        }
        else
        {
            tx_done->len = qspi_align4(qspi_prepare_default_tx(tx_done->data, QSPI_TX_BUF_LEN));
        }

        xSemaphoreTake(qspi_stats_mutex, portMAX_DELAY);
        qspi_stats.tx_count++;
        xSemaphoreGive(qspi_stats_mutex);

        qspi_update_shared_regs();
        ESP_ERROR_CHECK(spi_slave_hd_queue_trans(QSPI_HOST, SPI_SLAVE_CHAN_TX, tx_done, portMAX_DELAY));
    }
}

/**********************************************************************************************************/
/**
 * @brief Update the shared registers that the master can read to get status and statistics information.
 *        This function is called after processing transactions to ensure the master has the latest data.
 */
static void qspi_update_shared_regs(void)
{
    xSemaphoreTake(qspi_stats_mutex, portMAX_DELAY);

    uint32_t value = QSPI_READY_FLAG;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_READY, (uint8_t *)&value, sizeof(value));

    value = qspi_stats.rx_count;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_RX_COUNT, (uint8_t *)&value, sizeof(value));

    value = qspi_stats.tx_count;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_TX_COUNT, (uint8_t *)&value, sizeof(value));

    value = qspi_stats.last_rx_len;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_LAST_RX_LEN, (uint8_t *)&value, sizeof(value));

    value = qspi_stats.last_rx_crc;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_LAST_RX_CRC, (uint8_t *)&value, sizeof(value));

    value = qspi_stats.test_pass;
    spi_slave_hd_write_buffer(QSPI_HOST, QSPI_REG_TEST_PASS, (uint8_t *)&value, sizeof(value));

    xSemaphoreGive(qspi_stats_mutex);
}

/**********************************************************************************************************/
/**
 * @brief Calculate the CRC16 checksum of the given data using the ESP ROM function.
 * @param data Pointer to the data buffer.
 * @param len Length of the data in bytes.
 * @return The calculated CRC16 checksum.
 */
static uint16_t qspi_crc16(const uint8_t *data, size_t len)
{
    if (data == NULL || len == 0)
    {
        return 0;
    }

    return esp_rom_crc16_le(0x1021, data, len);
}

/**********************************************************************************************************/
/**
 * @brief Align the given value to the next multiple of 4.
 *        This is required for DMA buffer alignment in SPI transactions.
 * @param value The value to align.
 * @return The aligned value, which is the smallest multiple of 4 that is greater than or equal to the input value.
 */
static size_t qspi_align4(size_t value)
{
    return (value + 3U) & (~3U);
}

/**********************************************************************************************************/
/**
 * @brief Prepare the default response to be sent to the master when there is no specific message in the queue.
 *        The response includes a status message with the current RX and TX counts.
 * @param out Pointer to the output buffer where the response will be written.
 * @param out_len Length of the output buffer in bytes.
 * @return The length of the prepared response in bytes, or 0 if there was an error.
 */
static size_t qspi_prepare_default_tx(uint8_t *out, size_t out_len)
{
    if (out == NULL || out_len == 0)
    {
        return 0;
    }

    int n = snprintf((char *)out,
                     out_len,
                     "QSPI_READY RX=%" PRIu32 " TX=%" PRIu32,
                     (uint32_t)qspi_stats.rx_count,
                     (uint32_t)qspi_stats.tx_count);

    if (n < 0)
    {
        return 0;
    }

    if ((size_t)n >= out_len)
    {
        return out_len;
    }

    return (size_t)n + 1U;
}

/**********************************************************************************************************/
/**
 * @brief Build the response to be sent back to the master based on the received command.
 *        If the received command matches the test command, it prepares a test response and updates the test pass status.
 *        Otherwise, it echoes back the received data with a prefix.
 * @param in Pointer to the input buffer containing the received data from the master.
 * @param in_len Length of the input data in bytes.
 * @param out Pointer to the output buffer where the response will be written.
 * @param out_len Length of the output buffer in bytes.
 * @return The length of the prepared response in bytes, or 0 if there was an error.
 */
static size_t qspi_build_rx_response(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
    if (out == NULL || out_len == 0)
    {
        return 0;
    }

    memset(out, 0, out_len);

    if (in != NULL && in_len >= QSPI_TEST_DATA_LEN && out_len >= QSPI_TEST_DATA_LEN &&
        in[0] == 'T' && in[1] == 'E' && in[2] == 'S' && in[3] == 'T')
    {
        out[0] = 'O';
        out[1] = 'K';
        out[2] = '6';
        out[3] = '4';
        for (size_t i = 4U; i < QSPI_TEST_DATA_LEN; ++i)
        {
            out[i] = (uint8_t)(0xA0U + (uint8_t)i);
        }

        xSemaphoreTake(qspi_stats_mutex, portMAX_DELAY);
        qspi_stats.test_pass = 1;
        xSemaphoreGive(qspi_stats_mutex);
        ESP_LOGI(TAG, "QSPI communication test PASS (fixed 64-byte response)");
        return QSPI_TEST_DATA_LEN;
    }

    size_t prefix_len = strlen(QSPI_TX_PREFIX);
    size_t copy_len = 0;

    if (in != NULL && in_len > 0)
    {
        copy_len = in_len;
        if (copy_len > (out_len - prefix_len - 1U))
        {
            copy_len = out_len - prefix_len - 1U;
        }
    }

    memcpy(out, QSPI_TX_PREFIX, prefix_len);
    if (copy_len > 0)
    {
        memcpy(out + prefix_len, in, copy_len);
    }

    out[prefix_len + copy_len] = '\0';
    return prefix_len + copy_len + 1U;
}

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/
