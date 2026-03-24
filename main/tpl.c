
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
#include "tpl.h"

/*
 * *********************************************************************************************************
 * Definitions and Declarations
 * *********************************************************************************************************
*/

#define TAG "TPL"

#define TPL_REG_WRITE(addr, data, len) spi_slave_hd_write_buffer(TPL_SPI, addr, (uint8_t *)data, len)
#define TPL_REG_READ(addr, out_data, len) spi_slave_hd_read_buffer(TPL_SPI, addr, out_data, len)

/*
 * *********************************************************************************************************
 * Global Variables
 * *********************************************************************************************************
*/

static volatile bool tpl_call_buffer_flag = false;
static volatile uint32_t tpl_rx_cb_count = 0;
static volatile uint32_t tpl_tx_cb_count = 0;
static tpl_regs_t tpl_regs = {0}; // Shared registers structure

/*
 * *********************************************************************************************************
 * Function Prototypes
 * *********************************************************************************************************
*/

static bool tpl_cb_buffer_rx(void *arg, spi_slave_hd_event_t *event, BaseType_t *awoken);
static bool tpl_cb_buffer_tx(void *arg, spi_slave_hd_event_t *event, BaseType_t *awoken);
static void tpl_task(void *pvParameters);

/*
 * *********************************************************************************************************
 * Function Implementations
 * *********************************************************************************************************
*/

/*
 * *********************************************************************************************************
 * @brief   Initialize the Transport Layer
 * @param   None
 * @return  None
*/
void tpl_init(void)
{
    // Configure IRQ output pin
    gpio_config_t irq_config = 
    {
        .pin_bit_mask = (1ULL << CONFIG_IRQ_OUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    // Configure and initialize IRQ output pin
    ESP_ERROR_CHECK(gpio_config(&irq_config));

    // Drive IRQ pin low initially
    ESP_ERROR_CHECK(gpio_set_level(CONFIG_IRQ_OUT_PIN, 0));

    // SPI slave bus configuration
    spi_bus_config_t bus_config = 
    {
        .sclk_io_num = CONFIG_SPI_SCK_PIN,
        .data0_io_num = CONFIG_SPI_D0_MOSI_PIN,
        .data1_io_num = CONFIG_SPI_D1_MISO_PIN,
        .data2_io_num = CONFIG_SPI_D2_PIN,
        .data3_io_num = CONFIG_SPI_D3_PIN,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .data_io_default_level = 0,
        .max_transfer_sz = TPL_FRAME_SIZE,
        .flags = SPICOMMON_BUSFLAG_SLAVE,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
    };

    // Initialize the SPI bus
    spi_slave_hd_slot_config_t slave_config = 
    {
        .mode = TPL_SPI_MODE,
        .spics_io_num = CONFIG_SPI_CS_PIN,
        .queue_size = TPL_QUEUE_SIZE,
        .command_bits = TPL_CMD_SIZE,
        .address_bits = TPL_REG_ADDRESS_SIZE,
        .dummy_bits = TPL_DUMMY_SIZE,
        .dma_chan = SPI_DMA_CH_AUTO,
        .flags = 0,
        .cb_config = 
        {
            .cb_buffer_rx = tpl_cb_buffer_rx,
            .cb_buffer_tx = tpl_cb_buffer_tx,
            .arg = NULL    
        },
    };

    // Initialize SPI slave in half-duplex mode
    ESP_ERROR_CHECK(spi_slave_hd_init(TPL_SPI, &bus_config, &slave_config));

    const uint8_t init_value[64] = {0};
    TPL_REG_WRITE(0, (uint8_t *)init_value, sizeof(init_value));
    ESP_LOGI(TAG, "Registers initialized with non-zero test pattern");

    // Create transport layer task
    xTaskCreate(tpl_task, "tpl_task", TPL_TASK_STACK_SIZE, NULL, TPL_TASK_PRIORITY, NULL);

    // Drive IRQ pin high at the end of initialization
    ESP_ERROR_CHECK(gpio_set_level(CONFIG_IRQ_OUT_PIN, 1));
}

/*
 * *********************************************************************************************************
 * Private Function Implementations
 * *********************************************************************************************************
*/

/*
 * @brief   SPI slave RX buffer callback
 * @param   arg User argument
 * @param   event SPI slave HD event information
 * @param   awoken Pointer to indicate if a higher priority task was woken
 * @return  bool True if a task was woken
*/
static bool tpl_cb_buffer_rx(void *arg, spi_slave_hd_event_t *event, BaseType_t *awoken)
{
    tpl_rx_cb_count++;
    tpl_call_buffer_flag = true;
    return false;
}

/*
 * *********************************************************************************************************
 * @brief   SPI slave TX buffer callback
 * @param   arg User argument
 * @param   event SPI slave HD event information
 * @param   awoken Pointer to indicate if a higher priority task was woken
 * @return  bool True if a task was woken
*/
static bool tpl_cb_buffer_tx(void *arg, spi_slave_hd_event_t *event, BaseType_t *awoken)
{
    tpl_tx_cb_count++;
    tpl_call_buffer_flag = true;
    return false;
}

/*
 * *********************************************************************************************************
 * @brief   Transport Layer Task
 * @param   pvParameters Task parameters
 * @return  None
*/
static void tpl_task(void *pvParameters)
{
    while (1)
    {
        if (tpl_call_buffer_flag)
        {
            tpl_call_buffer_flag = false;
            
            // Read all shared registers into local buffer
            TPL_REG_READ(0, (uint8_t *)&tpl_regs, sizeof(tpl_regs));
            
            // Print received data
            ESP_LOGI(TAG, "=== Shared Buffer Event (rx_cb=%lu tx_cb=%lu) ===", tpl_rx_cb_count, tpl_tx_cb_count);
    
            char hex_str[100] = {0};
            char *p = hex_str;
            for (size_t j = 0; j < 8; j++)
            {
                p += sprintf(p, "%02X ", ((uint8_t *)&tpl_regs)[j]);
            }
            ESP_LOGI(TAG, "[%02X-%02X]: %s", 0, 7, hex_str);
         
            ESP_LOGI(TAG, "===========================================");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
 * *********************************************************************************************************
 * End of File
 * *********************************************************************************************************
*/
