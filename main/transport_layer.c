/*
 * @file        transport_layer.c
 * @brief       SPI transport layer for ESP32 HOST project
 * @author      Nima Askari
 * @version     1.0.0
 * @license     See the LICENSE file in the root folder
 */

#include "transport_layer.h"
#include "esp_log.h"
#include "esp_random.h"
#include "driver/spi_slave_hd.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

#define TAG "TPL"

/* Configuration - these should match your hardware setup */
#define TPL_SPI_HOST             SPI2_HOST
#define TPL_SPI_QUEUE_SIZE       6
#define TPL_FRAME_SIZE           2048
#define TPL_SHARED_REG_SIZE      64
#define TPL_TASK_STACK_SIZE      4096
#define TPL_TASK_PRIORITY        1
#define TPL_CMD_FRAME_LEN        5

/* Append mode transaction management - follows official example */
#define QUEUE_SIZE               6
#define TRANSACTION_LEN          256
#define RX_PRELOAD_COUNT         1
#define TX_PRELOAD_COUNT         0

/* Register offsets for synchronization with master */
#define SYNC_REG_FROM_HOST       (14 * 4)  /* RX buffer count for master to read */
#define SYNC_REG_TO_HOST         (15 * 4)  /* TX bytes loaded for master to read */

/* Transaction link structure - matches official pattern */
typedef struct trans_link_s {
    spi_slave_hd_data_t trans;
    struct trans_link_s *next;
    bool recycled;  /* 1: available for reuse, 0: processing */
} trans_link_t;

/* Current transaction pointers */
static trans_link_t *tx_curr_trans = NULL;
static trans_link_t *rx_curr_trans = NULL;
static SemaphoreHandle_t tpl_append_mutex = NULL;
static volatile bool tpl_tx_request_pending = false;
static volatile uint8_t tpl_tx_request_reg = 0;
static volatile uint16_t tpl_tx_request_len = 1;

/* Forward declarations */
static void tx_task(void *arg);
static void rx_task(void *arg);
static void tpl_spi_init(void);

/*
 * ================================================
 * HELPER FUNCTIONS
 * ================================================
 */

static esp_err_t create_transaction_pool(uint8_t **data_buf, trans_link_t *trans_link, uint16_t times)
{
    for (int i = 0; i < times; i++) {
        /* Allocate DMA-safe buffer */
        data_buf[i] = spi_bus_dma_memory_alloc(TPL_SPI_HOST, TRANSACTION_LEN, 0);
        if (!data_buf[i]) {
            ESP_LOGE(TAG, "Failed to allocate buffer %d", i);
            return ESP_ERR_NO_MEM;
        }

        /* Attach data buffer to transaction descriptor */
        trans_link[i].trans.data = data_buf[i];

        /* Link in circular queue */
        if (i != QUEUE_SIZE - 1) {
            trans_link[i].next = &trans_link[i + 1];
        } else {
            trans_link[i].next = &trans_link[0];
        }

        /* Mark as available */
        trans_link[i].recycled = 1;

        ESP_LOGI(TAG, "Buffer %d allocated at %p", i, data_buf[i]);
    }
    return ESP_OK;
}

static bool get_tx_transaction_descriptor(trans_link_t **out_trans)
{
    if (tx_curr_trans->recycled == 0) {
        return false;
    }
    *out_trans = tx_curr_trans;
    tx_curr_trans = tx_curr_trans->next;
    return true;
}

static bool get_rx_transaction_descriptor(trans_link_t **out_trans)
{
    if (rx_curr_trans->recycled == 0) {
        return false;
    }
    rx_curr_trans->recycled = 0;
    rx_curr_trans->trans.len = TRANSACTION_LEN;
    *out_trans = rx_curr_trans;
    rx_curr_trans = rx_curr_trans->next;
    return true;
}

static esp_err_t queue_trans_retry(spi_slave_chan_t channel, spi_slave_hd_data_t *trans)
{
    const int max_retry = 25;
    for (int attempt = 0; attempt < max_retry; attempt++) {
        if (xSemaphoreTake(tpl_append_mutex, portMAX_DELAY) != pdTRUE) {
            return ESP_FAIL;
        }

        esp_err_t ret = spi_slave_hd_queue_trans(TPL_SPI_HOST, channel, trans, pdMS_TO_TICKS(10));
        xSemaphoreGive(tpl_append_mutex);

        if (ret == ESP_OK) {
            return ESP_OK;
        }
        if (ret != ESP_ERR_INVALID_STATE && ret != ESP_ERR_TIMEOUT) {
            return ret;
        }

        /* Driver reports DMA busy/queue full on some targets; retry shortly. */
        vTaskDelay(pdMS_TO_TICKS(4));
    }

    return ESP_ERR_TIMEOUT;
}

static void prepare_tx_data(trans_link_t *tx_trans)
{
    uint16_t req_len = tpl_tx_request_len;
    uint8_t req_reg = tpl_tx_request_reg;

    if (req_len == 0 || req_len > TRANSACTION_LEN) {
        req_len = 1;
    }
    if ((size_t)req_reg + req_len > TPL_SHARED_REG_SIZE) {
        req_len = (uint16_t)(TPL_SHARED_REG_SIZE - req_reg);
    }

    spi_slave_hd_read_buffer(TPL_SPI_HOST, req_reg, tx_trans->trans.data, req_len);
    tx_trans->trans.len = req_len;
    tx_trans->recycled = 0;
}

static void handle_rx_frame(const uint8_t *frame, size_t len)
{
    if (!frame || len < 2) {
        return;
    }

    uint8_t cmd = frame[0];
    uint8_t reg = frame[1];

    if (cmd == TPL_CMD_WRBUF || cmd == TPL_CMD_WRDMA) {
        /* Frame format: [WRBUF, reg, dummy, payload...] */
        size_t payload_start = (len >= 3) ? 3 : 2;

        if (len <= payload_start) {
            ESP_LOGW(TAG, "WRBUF has no payload");
            return;
        }

        size_t payload_len = len - payload_start;
        if ((size_t)reg + payload_len > TPL_SHARED_REG_SIZE) {
            ESP_LOGW(TAG, "WRBUF out of range: reg=%u len=%u", reg, (unsigned)payload_len);
            return;
        }

        spi_slave_hd_write_buffer(TPL_SPI_HOST, reg, (uint8_t *)&frame[payload_start], payload_len);
        ESP_LOGI(TAG, "%s reg=0x%02X len=%u",
                 (cmd == TPL_CMD_WRDMA) ? "WRDMA" : "WRBUF",
                 reg, (unsigned)payload_len);
        return;
    }

    if (cmd == TPL_CMD_RDBUF || cmd == TPL_CMD_RDDMA) {
        uint16_t req_len = 1;

        /* Read frame formats supported:
         * 1) [RDBUF, reg, dummy, <dummy...>] -> read length = trailing bytes
         * 2) [RDBUF, reg, len] (legacy)
         */
        if (len > 3) {
            req_len = (uint16_t)(len - 3);
        } else if (len == 3 && frame[2] > 0) {
            req_len = frame[2];
        }

        if ((size_t)reg + req_len > TPL_SHARED_REG_SIZE) {
            req_len = (uint16_t)(TPL_SHARED_REG_SIZE - reg);
        }

        tpl_tx_request_reg = reg;
        tpl_tx_request_len = req_len;
        tpl_tx_request_pending = true;
        ESP_LOGI(TAG, "%s request reg=0x%02X len=%u",
                 (cmd == TPL_CMD_RDDMA) ? "RDDMA" : "RDBUF",
                 reg, (unsigned)req_len);
        return;
    }

    ESP_LOGW(TAG, "Unknown cmd=0x%02X len=%u", cmd, (unsigned)len);
}

/*
 * ================================================
 * PUBLIC API
 * ================================================
 */

void tpl_init(void)
{
    ESP_LOGI(TAG, "Transport layer initializing...");

    tpl_spi_init();

    tpl_append_mutex = xSemaphoreCreateMutex();
    if (tpl_append_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create append mutex");
        return;
    }

    /* Initialize shared registers to zero at startup. */
    uint8_t init_value[SOC_SPI_MAXIMUM_BUFFER_SIZE] = {0};
    uint32_t sync_init = 0;
    spi_slave_hd_write_buffer(TPL_SPI_HOST, 0, init_value, SOC_SPI_MAXIMUM_BUFFER_SIZE);
    spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_FROM_HOST, (uint8_t *)&sync_init, sizeof(sync_init));
    spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_TO_HOST, (uint8_t *)&sync_init, sizeof(sync_init));
    ESP_LOGI(TAG, "Shared registers initialized to zero");

    /* Wait for SPI hardware to be ready before queuing transactions */
    vTaskDelay(pdMS_TO_TICKS(100));
   
    
    /* Create RX first, then TX to avoid simultaneous preload contention */
    xTaskCreate(rx_task, "rx_task", TPL_TASK_STACK_SIZE, NULL, TPL_TASK_PRIORITY, NULL);
    vTaskDelay(pdMS_TO_TICKS(30));
    xTaskCreate(tx_task, "tx_task", TPL_TASK_STACK_SIZE, NULL, TPL_TASK_PRIORITY, NULL);
    
    ESP_LOGI(TAG, "Transport layer initialized with TX/RX tasks");
}

void tpl_shared_reg_write(uint32_t addr, uint8_t *data, size_t len)
{
    if (addr + len > TPL_SHARED_REG_SIZE || !data || len == 0 || len > TRANSACTION_LEN)
    {
        ESP_LOGE(TAG, "shared_reg_write: Invalid parameters");
        return;
    }
    /* Write to SPI slave shared register */
    spi_slave_hd_write_buffer(TPL_SPI_HOST, addr, data, len);
}

void tpl_shared_reg_read(uint32_t addr, uint8_t *data, size_t len)
{
    if (addr + len > TPL_SHARED_REG_SIZE || !data || len == 0 || len > TRANSACTION_LEN)
    {
        ESP_LOGE(TAG, "shared_reg_read: Invalid parameters");
        return;
    }
    /* Read from SPI slave shared register */
    spi_slave_hd_read_buffer(TPL_SPI_HOST, addr, data, len);
}

bool tpl_append_tx_buffer(uint32_t addr, uint8_t *data, size_t len)
{
    /* This API is replaced by TX task - not used in append mode */
    (void)addr;
    (void)data;
    (void)len;
    ESP_LOGW(TAG, "append_tx_buffer: Use TX task for append mode");
    return false;
}

bool tpl_append_rx_buffer(uint32_t addr, uint8_t *data, size_t len)
{
    /* This API is replaced by RX task - not used in append mode */
    (void)addr;
    (void)data;
    (void)len;
    ESP_LOGW(TAG, "append_rx_buffer: Use RX task for append mode");
    return false;
}


/*
 * ================================================
 * PRIVATE FUNCTIONS
 * ================================================
 */

static void tpl_spi_init(void)
{
    ESP_LOGI(TAG, "Initializing SPI slave HD...");

    gpio_config_t cs_cfg = {
        .pin_bit_mask = 1ULL << CONFIG_COM_SPI_CS_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&cs_cfg));
    ESP_LOGI(TAG, "CS configured active-low (idle high with pull-up) on GPIO %d", CONFIG_COM_SPI_CS_PIN);

    gpio_config_t data_cfg = {
        .pin_bit_mask = (1ULL << CONFIG_COM_SPI_SCK_PIN) |
                        (1ULL << CONFIG_COM_SPI_D0_MOSI_PIN) |
                        (1ULL << CONFIG_COM_SPI_D1_MISO_PIN) |
                        (1ULL << CONFIG_COM_SPI_D2_PIN) |
                        (1ULL << CONFIG_COM_SPI_D3_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&data_cfg));
    ESP_LOGI(TAG, "SPI data lines pulled down for stable idle: SCK=%d D0=%d D1=%d D2=%d D3=%d",
        CONFIG_COM_SPI_SCK_PIN,
        CONFIG_COM_SPI_D0_MOSI_PIN,
        CONFIG_COM_SPI_D1_MISO_PIN,
        CONFIG_COM_SPI_D2_PIN,
        CONFIG_COM_SPI_D3_PIN);
    
    spi_bus_config_t buscfg = {
        .data0_io_num = CONFIG_COM_SPI_D0_MOSI_PIN,
        .data1_io_num = CONFIG_COM_SPI_D1_MISO_PIN,
        .data2_io_num = CONFIG_COM_SPI_D2_PIN,
        .data3_io_num = CONFIG_COM_SPI_D3_PIN,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .sclk_io_num = CONFIG_COM_SPI_SCK_PIN,
        .max_transfer_sz = TPL_FRAME_SIZE,
        .flags = SPICOMMON_BUSFLAG_QUAD,
        .intr_flags = 0,
    };

    spi_slave_hd_slot_config_t slotcfg = {
        .spics_io_num = CONFIG_COM_SPI_CS_PIN,
        .flags = 0,
        .mode = 0,
        .command_bits = 8,
        .address_bits = 8,
        .dummy_bits = 8,
        .queue_size = TPL_SPI_QUEUE_SIZE,
        .dma_chan = SPI_DMA_CH_AUTO,
    };

    /* Initialize SPI slave HD */
    ESP_ERROR_CHECK(spi_slave_hd_init(TPL_SPI_HOST, &buscfg, &slotcfg));
   
}

/*
 * ================================================
 * TX TASK
 * ================================================
 */

static void tx_task(void *arg)
{
    (void)arg;

    uint8_t *tx_buffer[QUEUE_SIZE] = {};
    trans_link_t trans_link[QUEUE_SIZE] = {};
    trans_link_t *trans_to_send;
    uint32_t total_load_buf_size = 0;
    bool tx_inflight = false;

    /* Initialize TX transaction pool */
    tx_curr_trans = trans_link;
    ESP_ERROR_CHECK(create_transaction_pool(tx_buffer, trans_link, QUEUE_SIZE));

    ESP_LOGI(TAG, "TX Task: Pre-loading %d transactions", TX_PRELOAD_COUNT);

    /* Pre-load all TX transactions to pipeline */
    for (int i = 0; i < TX_PRELOAD_COUNT; i++) {
        if (get_tx_transaction_descriptor(&trans_to_send)) {
            prepare_tx_data(trans_to_send);
            esp_err_t ret = queue_trans_retry(SPI_SLAVE_CHAN_TX, &trans_to_send->trans);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "TX preload append failed on buf %d: %s", i, esp_err_to_name(ret));
                trans_to_send->recycled = 1;
                vTaskDelay(pdMS_TO_TICKS(5));
                break;
            }
            total_load_buf_size += TRANSACTION_LEN;
            spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_TO_HOST, 
                                     (uint8_t *)&total_load_buf_size, 4);
            ESP_LOGI(TAG, "TX pre-loaded: buf %d, total bytes=%u", i, total_load_buf_size);
        }
    }

    ESP_LOGI(TAG, "TX Task: Starting main loop");

    /* Main TX loop - queue only when an RDBUF request arrives. */
    while (1) {
        spi_slave_hd_data_t *ret_trans;
        trans_link_t *ret_link;

        if (!tpl_tx_request_pending) {
            vTaskDelay(pdMS_TO_TICKS(2));
            continue;
        }

        if (!tx_inflight && get_tx_transaction_descriptor(&trans_to_send)) {
            prepare_tx_data(trans_to_send);
            esp_err_t ret = queue_trans_retry(SPI_SLAVE_CHAN_TX, &trans_to_send->trans);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "TX append failed in loop: %s", esp_err_to_name(ret));
                trans_to_send->recycled = 1;
                vTaskDelay(pdMS_TO_TICKS(5));
                continue;
            }
            tx_inflight = true;
            total_load_buf_size += trans_to_send->trans.len;
            spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_TO_HOST, 
                                     (uint8_t *)&total_load_buf_size, 4);
        }

        if (!tx_inflight) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        /* Get completed TX transaction (blocking) */
        ESP_ERROR_CHECK(spi_slave_hd_get_trans_res(TPL_SPI_HOST, SPI_SLAVE_CHAN_TX,
                                                   &ret_trans, portMAX_DELAY));

        /* Find the original link structure from the returned descriptor */
        ret_link = __containerof(ret_trans, trans_link_t, trans);
        ret_link->recycled = 1;
        tx_inflight = false;
        tpl_tx_request_pending = false;

        ESP_LOGI(TAG, "TX completed: %zu bytes sent", ret_trans->len);
    }
}


/*
 * ================================================
 * RX TASK
 * ================================================
 */

static void rx_task(void *arg)
{
    (void)arg;

    uint8_t *rx_buffer[QUEUE_SIZE] = {};
    trans_link_t trans_link[QUEUE_SIZE] = {};
    trans_link_t *trans_for_recv;
    uint32_t total_load_buf_num = 0;
    uint8_t last_cmd_frame[TPL_CMD_FRAME_LEN] = {0};

    /* Initialize RX transaction pool */
    rx_curr_trans = trans_link;
    ESP_ERROR_CHECK(create_transaction_pool(rx_buffer, trans_link, QUEUE_SIZE));

    ESP_LOGI(TAG, "RX Task: Pre-loading %d transactions", RX_PRELOAD_COUNT);

    /* Pre-load all RX transactions to pipeline */
    for (int i = 0; i < RX_PRELOAD_COUNT; i++) {
        if (get_rx_transaction_descriptor(&trans_for_recv)) {
            esp_err_t ret = queue_trans_retry(SPI_SLAVE_CHAN_RX, &trans_for_recv->trans);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "RX preload append failed on buf %d: %s", i, esp_err_to_name(ret));
                trans_for_recv->recycled = 1;
                vTaskDelay(pdMS_TO_TICKS(5));
                break;
            }
            total_load_buf_num += 1;
            spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_FROM_HOST, 
                                     (uint8_t *)&total_load_buf_num, 4);
            ESP_LOGI(TAG, "RX pre-loaded: buf %d, total count=%u", i, total_load_buf_num);
        }
    }

    ESP_LOGI(TAG, "RX Task: Starting main loop");

    /* Main RX loop - get result and queue new transaction */
    while (1)
    {
        spi_slave_hd_data_t *ret_trans;
        trans_link_t *ret_link;
        esp_err_t rx_ret;

        /* Poll command mailbox (WRBUF/RDBUF path) without driver callbacks. */
        {
            uint8_t cmd_frame[TPL_CMD_FRAME_LEN] = {0};
            spi_slave_hd_read_buffer(TPL_SPI_HOST, 0, cmd_frame, sizeof(cmd_frame));
            if (memcmp(cmd_frame, last_cmd_frame, sizeof(cmd_frame)) != 0) {
                memcpy(last_cmd_frame, cmd_frame, sizeof(cmd_frame));
                if (cmd_frame[0] == TPL_CMD_WRBUF || cmd_frame[0] == TPL_CMD_RDBUF ||
                    cmd_frame[0] == TPL_CMD_WRDMA || cmd_frame[0] == TPL_CMD_RDDMA) {
                    handle_rx_frame(cmd_frame, sizeof(cmd_frame));
                }
            }
        }

        /* Poll DMA RX transaction results (WRDMA/RDDMA data path). */
        rx_ret = spi_slave_hd_get_trans_res(TPL_SPI_HOST, SPI_SLAVE_CHAN_RX,
                                             &ret_trans, pdMS_TO_TICKS(20));

        if (rx_ret == ESP_OK && ret_trans != NULL)
        {
            /* Find the original link structure from the returned descriptor */
            ret_link = __containerof(ret_trans, trans_link_t, trans);
            ret_link->recycled = 1;

            ESP_LOGI(TAG, "RX completed: %zu bytes received", ret_trans->trans_len);
            ESP_LOG_BUFFER_HEX(TAG "slave RX", ret_trans->data, ret_trans->trans_len);
            handle_rx_frame(ret_trans->data, ret_trans->trans_len);

            /* Get new descriptor and queue if available */
            if (get_rx_transaction_descriptor(&trans_for_recv)) {
                esp_err_t ret = queue_trans_retry(SPI_SLAVE_CHAN_RX, &trans_for_recv->trans);
                if (ret != ESP_OK) {
                    ESP_LOGE(TAG, "RX append failed in loop: %s", esp_err_to_name(ret));
                    vTaskDelay(pdMS_TO_TICKS(5));
                } else {
                    total_load_buf_num += 1;
                    spi_slave_hd_write_buffer(TPL_SPI_HOST, SYNC_REG_FROM_HOST,
                                             (uint8_t *)&total_load_buf_num, 4);
                }
            }
        }

        /* Data is printed on every successful RX completion above. */
    }
}
