/*
 * @file        transport_layer.h
 * @brief       SPI transport layer for ESP32 HOST project
 * @author      Nima Askari
 * @version     1.0.0
 * @license     See the LICENSE file in the root folder
 */

#ifndef TRANSPORT_LAYER_H
#define TRANSPORT_LAYER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Commands from master */
#define TPL_CMD_WRBUF            0x01    /* Write to slave buffer (append mode) */
#define TPL_CMD_RDBUF            0x02    /* Read from slave buffer (append mode) */
#define TPL_CMD_WRDMA            0x03    /* Write via DMA */
#define TPL_CMD_RDDMA            0x04    /* Read via DMA */


/*
 * ================================================
 * TRANSPORT LAYER PUBLIC FUNCTIONS
 * ================================================
 */

/**
 * @brief Initialize the SPI slave transport layer
 * @details Sets up SPI slave HD with append mode, configures buffers, and starts monitoring task
 * @return void
 */
void tpl_init(void);

/**
 * @brief Write data to shared register space (register write)
 * @param addr Register address (0-63)
 * @param data Pointer to data
 * @param len Length in bytes
 */
void tpl_shared_reg_write(uint32_t addr, uint8_t *data, size_t len);

/**
 * @brief Read data from shared register space (register read)
 * @param addr Register address (0-63)
 * @param data Pointer to buffer
 * @param len Length in bytes
 */
void tpl_shared_reg_read(uint32_t addr, uint8_t *data, size_t len);

/**
 * @brief Append data to TX buffer (WRBUF command)
 * @details Appends data to the SPI TX buffer for master to read
 * @param addr Register address or offset
 * @param data Pointer to data to transmit
 * @param len Length in bytes
 * @return true if appended successfully, false otherwise
 */
bool tpl_append_tx_buffer(uint32_t addr, uint8_t *data, size_t len);

/**
 * @brief Append RX buffer for receiving data (RDBUF command)
 * @details Appends a buffer to receive data from SPI master
 * @param addr Register address or offset
 * @param data Pointer to buffer for received data
 * @param len Length in bytes
 * @return true if appended successfully, false otherwise
 */
bool tpl_append_rx_buffer(uint32_t addr, uint8_t *data, size_t len);

#endif /* TRANSPORT_LAYER_H */
