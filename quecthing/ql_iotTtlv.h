/*
Copyright (C) 2023  Quectel Wireless Solutions Co.,Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef __QL_IOTTTLV_H__
#define __QL_IOTTTLV_H__

#include "Qhal_types.h"
#include "quos_cjson.h"
typedef enum
{
    QIOT_DPDATA_TYPE_BOOL = 0,
    QIOT_DPDATA_TYPE_INT,
    QIOT_DPDATA_TYPE_FLOAT,
    QIOT_DPDATA_TYPE_BYTE,
    QIOT_DPDATA_TYPE_STRUCT,
} QIot_dpDataType_e;

enum
{
    QIOT_MBID_PK = 1,
    QIOT_MBID_PS = 2,
    QIOT_MBID_REPORT  = 3,
    QIOT_MBID_VER = 4,
    QIOT_MBID_UARTCFG = 5,
    QIOT_MBID_MODELCFG = 6,
    QIOT_MBID_DEVCFG = 7,
    QIOT_MBID_UART_PORT = 8,
    QIOT_MBID_UART_BAUDRATE = 9,
    QIOT_MBID_UART_DATABITS = 10,
    QIOT_MBID_UART_STOPBITS = 11,
    QIOT_MBID_UART_PARITY = 12,
    QIOT_MBID_UART_POLLITV = 13,
    QIOT_MBID_UART_CMDITV = 14,
    QIOT_MBID_MODEL_ID = 15,
    QIOT_MBID_MODEL_TYPE = 16,
    QIOT_MBID_MODEL_BYTEORDER = 17,
    QIOT_MBID_MODEL_MULTIPLE = 18,
    QIOT_MBID_MODEL_INCREMENT = 19,
    QIOT_MBID_MODEL_LEN = 20,
    QIOT_MBID_DEV_ADDR = 21,
    QIOT_MBID_DEV_DATA = 22,
    QIOT_MBID_ADDR_SLAVE = 23,
    QIOT_MBID_SLAVE_ADDR = 24,
    QIOT_MBID_SLAVE_ID = 25,
    QIOT_MBID_DATA_0X01 = 26,
    QIOT_MBID_DATA_0X03 = 27,
    QIOT_MBID_DATA_0X04 = 28,
    QIOT_MBID_DATA_0X05 = 29,
    QIOT_MBID_DATA_0X06 = 30,
    QIOT_MBID_DATA_0X0F = 31,
    QIOT_MBID_DATA_0X10 = 32,
    QIOT_MBID_REG_ADDR = 33,
    QIOT_MBID_REG_NUM = 34,
    QIOT_MBID_REG_DATA = 35,
    QIOT_MBID_TSLID = 36,
    QIOT_MBID_REG_BYTEOFFSET = 37,
    QIOT_MBID_REG_BITOFFSET = 38,
    QIOT_MBID_SUB = 39,
    QIOT_MBID_SUB_PK = 40,
    QIOT_MBID_SUB_PS = 41,
    QIOT_MBID_UART_RESENDCOUNT = 42,
    QIOT_MBID_ATTR_TYPE = 43,
    QIOT_MBID_EVENT_INFO = 44,
    QIOT_MBID_EVENT_ID = 45,
    QIOT_MBID_EVENT_BOOL_TRIGGER = 46,
    QIOT_MBID_EVENT_MUN_MINI = 47,
    QIOT_MBID_EVENT_MUN_MAX = 48,
    QIOT_MBID_EVENT_BYTE_TRIGGER = 49,
    QIOT_MBID_EVENT_ENUM_TRIGGER = 50,
    QIOT_MBID_DATA_0X02 = 51,
    QIOT_MBID_DEV_TYPE = 52,
};

void *Ql_iotTtlvUnformat(const quint8_t *buffer, quint32_t len);
quint32_t Ql_iotTtlvFormatLen(const void *ttlvHead);
quint32_t Ql_iotTtlvFormat(const void *ttlvHead, quint8_t *retBuf);

/* JSON和TTLV 互转 */
void *Ql_iotJson2Ttlv(const cJSON *json);
cJSON *Ql_iotTtlv2Json(const void *ttlvHead);
#endif
