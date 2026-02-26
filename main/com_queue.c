
/*
 * @file        com_queue.c
 * @brief       Event queue implementation for communication module
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

/*
 * ********************************************************************************************************
 * INCLUDES
 * ********************************************************************************************************
*/
#include "com.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>
#include <assert.h>

/*
 * ********************************************************************************************************
 * DEFINES
 * ********************************************************************************************************
*/

#define TAG "COM_QUEUE"

/*
 * ********************************************************************************************************
 * VARIABLES
 * ********************************************************************************************************
*/

static QueueHandle_t com_event_queue = NULL;

/*
 * ********************************************************************************************************
 * FUNCTION IMPLEMENTATION
 * ********************************************************************************************************
*/

/**********************************************************************************************************/
/**
 * @brief   Initializes the communication event queue.
 * @details This function creates a FreeRTOS queue to hold communication events.
 *      It should be called during the initialization of the communication module.
 * @return  None
 */
void com_event_queue_init(void)
{
	com_event_queue = xQueueCreate(COM_EVENT_QUEUE_LEN, sizeof(com_event_frame_t));
	if (com_event_queue == NULL)
	{
		ESP_LOGE(TAG, "Failed to create event queue");
	}
}

/**********************************************************************************************************/
/**
 * @brief   Pushes an event to the communication event queue.
 * @details This function adds a new event to the FreeRTOS queue. If the queue is full, the event is dropped.
 * @param   type  The type of the event.
 * @param   param Additional parameter for the event.
 * @param   data  Pointer to the event data.
 * @param   len   Length of the event data.
 * @return  None
 */
void com_event_push(uint8_t type, uint8_t param, const uint8_t *data, uint16_t len)
{
	assert(com_event_queue != NULL);

	com_event_frame_t frame = {0};
	frame.type = type;
	frame.param = param;
	frame.len = (len > sizeof(frame.data)) ? sizeof(frame.data) : len;
	
	if (data && frame.len > 0)
	{
		memcpy(frame.data, data, frame.len);
	}

	if (xQueueSend(com_event_queue, &frame, 0) != pdTRUE)
	{
		ESP_LOGW(TAG, "Event queue full, dropping event");
	}
}

/**********************************************************************************************************/
/**
 * @brief   Pops an event from the communication event queue.
 * @details This function retrieves an event from the FreeRTOS queue. If the queue is empty, it returns false.
 * @param   type  Pointer to store the event type.  
 * @param   param Pointer to store the event parameter.
 * @param   data  Pointer to store the event data.
 * @param   len   Pointer to store the length of the event data.
 * @return  True if an event was retrieved, false otherwise.
 */
bool com_event_pop(uint8_t *type, uint8_t *param, uint8_t *data, uint16_t *len)
{
	assert(com_event_queue != NULL);
	assert(type != NULL);
	assert(param != NULL);
	assert(data != NULL);
	assert(len != NULL);

	com_event_frame_t frame = {0};
	if (xQueueReceive(com_event_queue, &frame, 0) == pdTRUE)
	{
		*type = frame.type;
		*param = frame.param;
		*len = frame.len;
		memcpy(data, frame.data, frame.len);
		return true;
	}

	return false;
}

/*
 * ********************************************************************************************************
 * END
 * ********************************************************************************************************
*/
