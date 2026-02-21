

idf_component_register(
    SRCS 
        "main.c"
        "com.c"

    INCLUDE_DIRS 
        "."
        
    PRIV_REQUIRES 
        driver
        esp_driver_gpio
        esp_driver_uart
        esp_driver_spi 
        esp_driver_sdio
        esp_ringbuf
        esp_wifi
        nvs_flash
)

littlefs_create_partition_image(storage ../flash_data FLASH_IN_PROJECT)
