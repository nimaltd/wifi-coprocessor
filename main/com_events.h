
/*
 * @file        com_events.h
 * @brief       Event type definitions for the communication module of the ESP32 HOST project.
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

#ifndef _COM_EVENTS_H_
#define _COM_EVENTS_H_

/*
 * ********************************************************************************************************
 * EVENT TYPE DEFINITIONS
 * ********************************************************************************************************
*/

/* WiFi Events */
#define COM_EVT_WIFI_CONNECTED          0x01
#define COM_EVT_WIFI_DISCONNECTED       0x02
#define COM_EVT_WIFI_SCANNING           0x03
#define COM_EVT_WIFI_SCAN_DONE          0x04

/* TCP Events */
#define COM_EVT_TCP_CONNECTED           0x10
#define COM_EVT_TCP_DISCONNECTED        0x11
#define COM_EVT_TCP_NEW_DATA            0x12
#define COM_EVT_TCP_SEND_DONE           0x13

/* UDP Events */
#define COM_EVT_UDP_CONNECTED           0x20
#define COM_EVT_UDP_DISCONNECTED        0x21
#define COM_EVT_UDP_NEW_DATA            0x22
#define COM_EVT_UDP_SEND_DONE           0x23

/* System Events */
#define COM_EVT_SYSTEM_ERROR            0xE0
#define COM_EVT_SYSTEM_INFO             0xE1
#define COM_EVT_SYSTEM_WARNING          0xE2

/*
 * ********************************************************************************************************
 * END
 * ********************************************************************************************************
*/

#endif /* _COM_EVENTS_H_ */
