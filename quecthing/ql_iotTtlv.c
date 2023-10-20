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
/*************************************************************************
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    : 
** 功能   @brief   : 
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "ql_iotTtlv.h"
#include "quos_twll.h"
#include "quos_cjson.h"
#include "quos_SupportTool.h"
#include <stdio.h>

/* 数据类型 */
enum
{
    DP_TTLV_TYPE_BOOL_FALSE = 0, /* 布尔值false */
    DP_TTLV_TYPE_BOOL_TRUE,      /* 布尔值true */
    DP_TTLV_TYPE_ENUM_NUM,       /* 枚举和数值 */
    DP_TTLV_TYPE_BYTE,           /* 二进制数据 */
    DP_TTLV_TYPE_STRUCT,         /* 结构体 */
};

typedef struct
{
    TWLLHead_T head;
    quint16_t id;
    QIot_dpDataType_e type;
    union
    {
        qbool vbool;
        double floatNum;
        qint64_t intNum;
        struct
        {
            quint8_t *val;
            quint16_t len;
        } vbytes;
        void *vstructHead;
    } value;
} QIot_ttlv_t;

void Ql_iotTtlvFree(void **ttlvHead);
/**************************************************************************
** 功能	@brief : 释放TTLV节点值内存
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotTtlvFreeNodeValue(QIot_ttlv_t node)
{
    if (QIOT_DPDATA_TYPE_BYTE == node.type)
    {
        HAL_FREE(node.value.vbytes.val);
    }
    else if (QIOT_DPDATA_TYPE_STRUCT == node.type)
    {
        Ql_iotTtlvFree(&node.value.vstructHead);
    }
}
/**************************************************************************
** 功能	@brief : 释放TTLV链表内存
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Ql_iotTtlvFree(void **ttlvHead)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((*(TWLLHead_T **)ttlvHead), temp, next)
    {
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        Quos_twllHeadDelete((TWLLHead_T **)ttlvHead, &node->head);
        ql_iotTtlvFreeNodeValue(*node);
        HAL_FREE(node);
    }
    *ttlvHead = NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t  Ql_iotTtlvCountGet(const void *ttlvHead)
{
    return Quos_twllHeadGetNodeCount((TWLLHead_T *)ttlvHead);
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void *Ql_iotTtlvNodeGet(const void *ttlvHead, quint16_t index, quint16_t *id, QIot_dpDataType_e *type)
{
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        if (0 == index)
        {
            QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
            if (id)
            {
                *id = node->id;
            }
            if (type)
            {
                *type = node->type;
            }
            return &node->head;
        }
        index--;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvNodeGetBool(const void *ttlvNode, qbool *value)
{
    if (NULL == ttlvNode)
    {
        return FALSE;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (value && QIOT_DPDATA_TYPE_BOOL == node->type)
    {
        *value = node->value.vbool;
        return TRUE;
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvNodeGetInt(const void *ttlvNode, qint64_t *value)
{
    if (NULL == ttlvNode)
    {
        return FALSE;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (value && QIOT_DPDATA_TYPE_INT == node->type)
    {
        *value = (int)node->value.intNum;
        return TRUE;
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvNodeGetFloat(const void *ttlvNode, double *value)
{
    if (NULL == ttlvNode)
    {
        return FALSE;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (value && QIOT_DPDATA_TYPE_FLOAT == node->type)
    {
        *value = node->value.floatNum;
        return TRUE;
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char *Ql_iotTtlvNodeGetString(const void *ttlvNode)
{
    if (NULL == ttlvNode)
    {
        return NULL;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (QIOT_DPDATA_TYPE_BYTE == node->type && node->value.vbytes.val && HAL_STRLEN(node->value.vbytes.val))
    {
        return (char *)node->value.vbytes.val;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Ql_iotTtlvNodeGetByte(const void *ttlvNode, quint8_t **value)
{
    if (NULL == ttlvNode)
    {
        return FALSE;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (value && QIOT_DPDATA_TYPE_BYTE == node->type)
    {
        *value = node->value.vbytes.val;
        return node->value.vbytes.len;
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void *Ql_iotTtlvNodeGetStruct(const void *ttlvNode)
{
    if (NULL == ttlvNode)
    {
        return NULL;
    }
    QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT((TWLLHead_T *)ttlvNode, QIot_ttlv_t, head);
    if (QIOT_DPDATA_TYPE_STRUCT == node->type)
    {
        return node->value.vstructHead;
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 根据ID查找TTLV节点
** 输入	@param :
** 输出	@retval: 
***************************************************************************/
static QIot_ttlv_t *ql_iotTtlvValIdGet(const void *ttlvHead, quint16_t id)
{
    if (0 == id)
    {
        return NULL;
    }
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        if (id == node->id)
        {
            return node;
        }
    }
    return NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdGetBool(const void *ttlvHead, quint16_t id, qbool *value)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetBool(&node->head, value) : FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool  Ql_iotTtlvIdGetInt(const void *ttlvHead, quint16_t id, qint64_t *value)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetInt(&node->head, value) : FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdGetFloat(const void *ttlvHead, quint16_t id, double *value)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetFloat(&node->head, value) : FALSE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
char *Ql_iotTtlvIdGetString(const void *ttlvHead, quint16_t id)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetString(&node->head) : NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Ql_iotTtlvIdGetByte(const void *ttlvHead, quint16_t id, quint8_t **value)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetByte(&node->head, value) : 0;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void *Ql_iotTtlvIdGetStruct(const void *ttlvHead, quint16_t id)
{
    QIot_ttlv_t *node = ql_iotTtlvValIdGet(ttlvHead, id);
    return node ? Ql_iotTtlvNodeGetStruct(&node->head) : NULL;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static QIot_ttlv_t *ql_iotTtlvGetNew(void **ttlvHead, quint16_t id, QIot_dpDataType_e type)
{
    QIot_ttlv_t *node = NULL;
    if (0 == id || (node = ql_iotTtlvValIdGet(*ttlvHead, id)) == NULL)
    {
        node = HAL_MALLOC(sizeof(QIot_ttlv_t));
        if (NULL == node)
        {
            return NULL;
        }
        HAL_MEMSET(node, 0, sizeof(QIot_ttlv_t));
        node->id = id;
        Quos_twllHeadAdd((TWLLHead_T **)ttlvHead, &node->head);
    }
    ql_iotTtlvFreeNodeValue(*node);
    node->type = type;
    return node;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddBool(void **ttlvHead, quint16_t id, qbool value)
{
    QIot_ttlv_t *node = ql_iotTtlvGetNew(ttlvHead, id, QIOT_DPDATA_TYPE_BOOL);
    if (NULL == node)
    {
        return FALSE;
    }
    node->value.vbool = value;
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddInt(void **ttlvHead, quint16_t id, qint64_t value)
{
    QIot_ttlv_t *node = ql_iotTtlvGetNew(ttlvHead, id, QIOT_DPDATA_TYPE_INT);
    if (NULL == node)
    {
        return FALSE;
    }
    node->value.intNum = value;
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddFloat(void **ttlvHead, quint16_t id, double value)
{
    QIot_ttlv_t *node = ql_iotTtlvGetNew(ttlvHead, id, QIOT_DPDATA_TYPE_FLOAT);
    if (NULL == node)
    {
        return FALSE;
    }
    node->value.floatNum = value;
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddByte(void **ttlvHead, quint16_t id, const quint8_t *data, quint32_t len)
{
    quint8_t *temp = HAL_MALLOC(len + 1); /* 预留1byte,在字符串类型时存储结束符 */
    if (NULL == temp)
    {
        return FALSE;
    }
    QIot_ttlv_t *node = ql_iotTtlvGetNew(ttlvHead, id, QIOT_DPDATA_TYPE_BYTE);
    if (NULL == node)
    {
        return FALSE;
    }
    HAL_MEMCPY(temp, data, len);
    temp[len] = 0;
    node->value.vbytes.val = temp;
    node->value.vbytes.len = len;
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddString(void **ttlvHead, quint16_t id, const char *data)
{
    return Ql_iotTtlvIdAddByte(ttlvHead, id, (const quint8_t *)data, HAL_STRLEN(data));
}
/**************************************************************************
** 功能	@brief : 
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Ql_iotTtlvIdAddStruct(void **ttlvHead, quint16_t id, void *vStruct)
{
    QIot_ttlv_t *node = ql_iotTtlvGetNew(ttlvHead, id, QIOT_DPDATA_TYPE_STRUCT);
    if (NULL == node)
    {
        return FALSE;
    }
    node->value.vstructHead = vStruct;
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 判断TTLV链表是否是数组结构体
** 输入	@param : 
** 输出	@retval: FALSE:链表错误
***************************************************************************/
static qbool  ql_iotTtlvIsStructArray(const void *ttlvHead, qbool *isArray)
{
    quint32_t arraySize = 0;
    if (isArray)
    {
        *isArray = FALSE;
    }
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        if (0 == node->id)
        {
            if (isArray)
            {
                *isArray = TRUE;
            }
            arraySize++;
        }
        else if (arraySize)
        {
            return FALSE;
        }
    }
    return TRUE;
}
/**************************************************************************
** 功能	@brief : 提取第一个TTLV
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static quint32_t  ql_iotTtlvUnformatFirst(const quint8_t *buffer, quint32_t len, QIot_ttlv_t *node)
{
    if (len < 2)
    {
        return 0;
    }
    HAL_MEMSET(node, 0, sizeof(QIot_ttlv_t));
    node->id = _ARRAY01_U16(&buffer[0]);
    quint8_t ttlvtype = node->id & 0x07;
    node->id >>= 3;
    switch (ttlvtype)
    {
    case DP_TTLV_TYPE_BOOL_FALSE:
    case DP_TTLV_TYPE_BOOL_TRUE:
    {
        node->type = QIOT_DPDATA_TYPE_BOOL;
        node->value.vbool = DP_TTLV_TYPE_BOOL_TRUE == ttlvtype ? TRUE : FALSE;
        return 2;
    }
    case DP_TTLV_TYPE_ENUM_NUM:
    {
        if (len < 3)
        {
            return 0;
        }
        qbool negative = (buffer[2] >> 7) ? TRUE : FALSE;
        quint8_t amp = (buffer[2] >> 3) & 0x0F;
        quint8_t tmpLen = ((buffer[2]) & 0x07) + 1;
        quint8_t offset;
        if (len < (quint32_t)(3 + tmpLen))
        {
            return 0;
        }
        qint64_t value = 0;
        for (offset = 0; offset < tmpLen; offset++)
        {
            value <<= 8;
            value |= buffer[offset + 3];
        }
        if (negative)
        {
            value = 0 - value;
        }
        if (amp)
        {
            node->type = QIOT_DPDATA_TYPE_FLOAT;
            node->value.floatNum = (double)value;
            while (amp--)
            {
                node->value.floatNum /= 10.0;
            }
        }
        else
        {
            node->type = QIOT_DPDATA_TYPE_INT;
            node->value.intNum = value;
        }
        return 3 + tmpLen;
    }
    case DP_TTLV_TYPE_BYTE:
    {
        if (len < 4)
        {
            return 0;
        }
        node->type = QIOT_DPDATA_TYPE_BYTE;
        node->value.vbytes.len = _ARRAY01_U16(&buffer[2]);
        if (len < (quint32_t)(4 + node->value.vbytes.len))
        {
            return 0;
        }
        node->value.vbytes.val = HAL_MALLOC(node->value.vbytes.len + 1);
        if (NULL == node->value.vbytes.val)
        {
            return 0;
        }
        HAL_MEMCPY(node->value.vbytes.val, &buffer[4], node->value.vbytes.len);
        node->value.vbytes.val[node->value.vbytes.len] = 0;
        return 4 + node->value.vbytes.len;
    }
    case DP_TTLV_TYPE_STRUCT:
    {
        if (len < 4)
        {
            return 0;
        }
        quint16_t count = _ARRAY01_U16(&buffer[2]);
        quint32_t offset = 4;
        node->type = QIOT_DPDATA_TYPE_STRUCT;
        while (count--)
        {
            QIot_ttlv_t *subNode = HAL_MALLOC(sizeof(QIot_ttlv_t));
            if (NULL == subNode)
            {
                return 0;
            }
            quint32_t nodeLen = ql_iotTtlvUnformatFirst(&buffer[offset], len - offset, subNode);
            if (nodeLen)
            {
                Quos_twllHeadAdd((TWLLHead_T **)&node->value.vstructHead, &subNode->head);
            }
            else
            {
                HAL_FREE(subNode);
                Ql_iotTtlvFree(&node->value.vstructHead);
                return 0;
            }
            offset += nodeLen;
        }
        if (FALSE == ql_iotTtlvIsStructArray(node->value.vstructHead, NULL))
        {
            Ql_iotTtlvFree(&node->value.vstructHead);
            return 0;
        }
        return offset;
    }
    default:
        return 0;
    }
}
/**************************************************************************
** 功能	@brief : 解压TTLV数据
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void *Ql_iotTtlvUnformat(const quint8_t *buffer, quint32_t len)
{
    void *ttlvHead = NULL;
    quint32_t offset;
    for (offset = 0; offset < len;)
    {
        QIot_ttlv_t *node = HAL_MALLOC(sizeof(QIot_ttlv_t));
        if (NULL == node)
        {
            return NULL;
        }
        quint32_t nodeLen = ql_iotTtlvUnformatFirst(&buffer[offset], len - offset, node);
        if (nodeLen)
        {
            offset += nodeLen;
            Quos_twllHeadAdd((TWLLHead_T **)&ttlvHead, &node->head);
        }
        else
        {
            HAL_FREE(node);
            Ql_iotTtlvFree((void **)&ttlvHead);
            return NULL;
        }
    }
    if (FALSE == ql_iotTtlvIsStructArray(ttlvHead, NULL))
    {
        Ql_iotTtlvFree(&ttlvHead);
    }
    return (void *)ttlvHead;
}
/**************************************************************************
** 功能	@brief : 计算构建TTLV数据包数据长度
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Ql_iotTtlvFormatLen(const void *ttlvHead)
{
    quint32_t pkgLen = 0;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        quint8_t tmpBuf[8];
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        pkgLen += 2;
        switch (node->type)
        {
        case QIOT_DPDATA_TYPE_BOOL: /*do none*/
            break;
        case QIOT_DPDATA_TYPE_INT:
        {
            pkgLen += 1;
            qint64_t value = node->value.intNum;
            if (value < 0)
            {
                value = 0 - value;
            }
            pkgLen += Quos_intPushArray((quint64_t)value, tmpBuf);
            break;
        }
        case QIOT_DPDATA_TYPE_FLOAT:
        {
            pkgLen += 1;
            volatile double value = node->value.floatNum;
            if (value < 0)
            {
                value = 0 - value;
            }
            volatile quint8_t amp = 0;
            while (0 != (value - (quint64_t)value) && amp < 0x0F)
            {
                value *= 10.0;
                amp++;
            }
            pkgLen += Quos_intPushArray((quint64_t)value, tmpBuf);
            break;
        }
        case QIOT_DPDATA_TYPE_BYTE:
        {
            pkgLen += 2 + node->value.vbytes.len;
            break;
        }
        case QIOT_DPDATA_TYPE_STRUCT:
        {
            pkgLen += 2 + Ql_iotTtlvFormatLen(node->value.vstructHead);
            break;
        }
        }
    }
    return pkgLen;
}
/**************************************************************************
** 功能	@brief : 构建TTLV数据包数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t Ql_iotTtlvFormat(const void *ttlvHead, quint8_t *retBuf)
{
    quint32_t pkgLen = 0;
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        _U16_ARRAY01(node->id << 3, &retBuf[pkgLen]);
        switch (node->type)
        {
        case QIOT_DPDATA_TYPE_BOOL:
        {
            retBuf[pkgLen + 1] |= node->value.vbool ? DP_TTLV_TYPE_BOOL_TRUE : DP_TTLV_TYPE_BOOL_FALSE;
            pkgLen += 2;
            break;
        }
        case QIOT_DPDATA_TYPE_INT:
        {
            qint64_t value = node->value.intNum;
            if (value < 0)
            {
                retBuf[pkgLen + 2] = 1 << 7;
                value = 0 - value;
            }
            else
            {
                retBuf[pkgLen + 2] = 0;
            }
            quint8_t tmpLen = Quos_intPushArray((quint64_t)value, &retBuf[pkgLen + 3]);
            retBuf[pkgLen + 1] |= DP_TTLV_TYPE_ENUM_NUM;
            retBuf[pkgLen + 2] |= (tmpLen - 1);
            pkgLen += 3 + tmpLen;
            break;
        }
        case QIOT_DPDATA_TYPE_FLOAT:
        {
            volatile double value = node->value.floatNum;
            if (value < 0)
            {
                retBuf[pkgLen + 2] = 1 << 7;
                value = 0 - value;
            }
            else
            {
                retBuf[pkgLen + 2] = 0;
            }

            volatile quint8_t amp = 0;
            while (0 != (value - (quint64_t)value) && amp < 0x0F)
            {
                value *= 10.0;
                amp++;
            }
            quint8_t tmpLen = Quos_intPushArray((quint64_t)value, &retBuf[pkgLen + 3]);
            retBuf[pkgLen + 1] |= DP_TTLV_TYPE_ENUM_NUM;
            retBuf[pkgLen + 2] |= (amp << 3) | (tmpLen - 1);
            pkgLen += 3 + tmpLen;
            break;
        }
        case QIOT_DPDATA_TYPE_BYTE:
        {
            retBuf[pkgLen + 1] |= DP_TTLV_TYPE_BYTE;
            _U16_ARRAY01(node->value.vbytes.len, &retBuf[pkgLen + 2]);
            HAL_MEMCPY(&retBuf[pkgLen + 4], node->value.vbytes.val, node->value.vbytes.len);
            pkgLen += 4 + node->value.vbytes.len;
            break;
        }
        case QIOT_DPDATA_TYPE_STRUCT:
        {
            quint16_t count = Quos_twllHeadGetNodeCount(node->value.vstructHead);
            retBuf[pkgLen + 1] |= DP_TTLV_TYPE_STRUCT;
            pkgLen += 2;
            _U16_ARRAY01(count, &retBuf[pkgLen]);
            pkgLen += 2;
            pkgLen += Ql_iotTtlvFormat(node->value.vstructHead, &retBuf[pkgLen]);
            break;
        }
        }
    }
    return pkgLen;
}

#define QL_IOTJSON_BYTE_MARK "\\x"
/**************************************************************************
** 功能	@brief : JSON转TTLV
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
static void ql_iotJson2Ttlv(const cJSON *json, void **ttlvHead)
{
    qbool ret = FALSE;
    quint32_t id = 0;
    if (json->string && FALSE == Quos_strIsUInt(json->string, HAL_STRLEN(json->string), &id))
    {
        return;
    }
    switch (json->type)
    {
    case QUOS_cJSON_False:
        ret = Ql_iotTtlvIdAddBool(ttlvHead, id, FALSE);
        break;
    case QUOS_cJSON_True:
        ret = Ql_iotTtlvIdAddBool(ttlvHead, id, TRUE);
        break;
    case QUOS_cJSON_Number:
        ret = Ql_iotTtlvIdAddFloat(ttlvHead, id, json->valuedouble);
        break;
    case QUOS_cJSON_String:
    case QUOS_cJSON_Raw:
    {
        if (0 == HAL_STRNCMP(json->valuestring, QL_IOTJSON_BYTE_MARK, HAL_STRLEN(QL_IOTJSON_BYTE_MARK)))
        {
            quint16_t vaildLen = HAL_STRLEN(json->valuestring + 2);
            quint8_t *tempBuf = HAL_MALLOC(vaildLen / 2);
            if (tempBuf)
            {
                if (vaildLen / 2 == Quos_str2Hex(json->valuestring + 2, tempBuf))
                {
                    ret = Ql_iotTtlvIdAddByte(ttlvHead, id, tempBuf, vaildLen / 2);
                }
                HAL_FREE(tempBuf);
            }
        }
        else if (0 == HAL_STRNCMP(json->valuestring, "\\\\", HAL_STRLEN("\\\\")))
        {
            ret = Ql_iotTtlvIdAddString(ttlvHead, id, json->valuestring + 1);
        }
        else
        {
            ret = Ql_iotTtlvIdAddString(ttlvHead, id, json->valuestring);
        }
    }

    break;
    case QUOS_cJSON_Array:
    case QUOS_cJSON_Object:
    {
        void *ttlvChild = NULL;
        cJSON *child = json->child;
        while (child)
        {
            ql_iotJson2Ttlv(child, &ttlvChild);
            if(NULL == ttlvChild)
            {
                break;
            }
            child = child->next;
        }
        if (ttlvChild && ql_iotTtlvIsStructArray(ttlvChild, NULL))
        {
            ret = Ql_iotTtlvIdAddStruct(ttlvHead, id, ttlvChild);
        }
        break;
    }
    default:
        break;
    }
    if(FALSE == ret)
    {
        Ql_iotTtlvFree(ttlvHead);
    }
}
/**************************************************************************
** 功能	@brief : JSON字符串转TTLV
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void *Ql_iotJson2Ttlv(const cJSON *json)
{
    if(NULL == json)
    {
        return NULL;
    }
    void *ttlvHead = NULL;
    cJSON *child = json->child;
    while (child)
    {
        ql_iotJson2Ttlv(child, &ttlvHead);
        if(NULL == ttlvHead)
        {
            break;
        }
        child = child->next;
    }
    return ttlvHead;
}
/**************************************************************************
** 功能	@brief : TTLV转JSON字符串
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
cJSON *Ql_iotTtlv2Json(const void *ttlvHead)
{
    qbool isArray;
    cJSON *jsonRoot = NULL;
    if (NULL == ttlvHead || FALSE == ql_iotTtlvIsStructArray(ttlvHead, &isArray))
    {
        return NULL;
    }
    TWLLHead_T *temp, *next;
    TWLIST_FOR_DATA((TWLLHead_T *)ttlvHead, temp, next)
    {
        QIot_ttlv_t *node = __GET_STRUCT_BY_ELEMENT(temp, QIot_ttlv_t, head);
        char id[10];
        HAL_SPRINTF(id, "%d", node->id);
        cJSON *jsonNode = NULL;
        switch (node->type)
        {
        case QIOT_DPDATA_TYPE_BOOL:
            jsonNode = cJSON_CreateBool(node->value.vbool);
            break;
        case QIOT_DPDATA_TYPE_INT:
            jsonNode = cJSON_CreateNumber(node->value.intNum);
            break;
        case QIOT_DPDATA_TYPE_FLOAT:
            jsonNode = cJSON_CreateNumber(node->value.floatNum);
            break;
        case QIOT_DPDATA_TYPE_BYTE:
        {
            quint16_t i;
            for (i = 0; i < node->value.vbytes.len; i++)
            {
                if (node->value.vbytes.val[i] < 0x20 || node->value.vbytes.val[i] > 0x7E)
                {
                    char *tempBuf = HAL_MALLOC(node->value.vbytes.len * 2 + HAL_STRLEN(QL_IOTJSON_BYTE_MARK) + 1);
                    if (tempBuf)
                    {
                        HAL_SPRINTF(tempBuf, "%s", QL_IOTJSON_BYTE_MARK);
                        Quos_hex2Str(node->value.vbytes.val, node->value.vbytes.len, tempBuf + HAL_STRLEN(tempBuf), TRUE);
                        jsonNode = cJSON_CreateString(tempBuf);
                        HAL_FREE(tempBuf);
                        break;
                    }
                }
            }
            if (i >= node->value.vbytes.len)
            {
                if(0 == HAL_STRNCMP(node->value.vbytes.val, "\\", HAL_STRLEN("\\")))
                {
                    char *tempBuf = HAL_MALLOC(node->value.vbytes.len+1+1);
                    if (tempBuf)
                    {
                        HAL_SPRINTF(tempBuf, "\\%s", node->value.vbytes.val);
                        jsonNode = cJSON_CreateString(tempBuf);
                        HAL_FREE(tempBuf);
                        break;
                    }
                }
                else
                {
                    jsonNode = cJSON_CreateString((const char*)node->value.vbytes.val);
                }
            }
        }
        break;
        case QIOT_DPDATA_TYPE_STRUCT:
            jsonNode = Ql_iotTtlv2Json(node->value.vstructHead);
            break;
        default:
            break;
        }
        if (NULL == jsonNode)
        {
            cJSON_Delete(jsonRoot);
            jsonRoot = NULL;
            break;
        }
        else if (isArray)
        {
            if (NULL == jsonRoot)
            {
                jsonRoot = cJSON_CreateArray();
            }
            cJSON_AddItemToArray(jsonRoot, jsonNode);
        }
        else
        {
            if (NULL == jsonRoot)
            {
                jsonRoot = cJSON_CreateObject();
            }
            cJSON_AddItemToObject(jsonRoot, id, jsonNode);
        }
    }
    return jsonRoot;
}
