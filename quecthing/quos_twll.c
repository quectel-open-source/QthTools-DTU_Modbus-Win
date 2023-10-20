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
**		    源码未经检录,使用需谨慎
** 创建人 @author  : 吴健超 JCWu
** 版本   @version : V1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   : 实现双向非循环向链表操作
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_twll.h"

/**************************************************************************
** 功能	@brief : 向链表尾部增加节点
** 输入	@param : list 链表的首节点地址 node 新链表节点
** 输出	@retval: 增加的节点指针
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadAdd(TWLLHead_T **list, TWLLHead_T *node)
{
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == node)
        {
            return;
        }
    }
    node->next = NULL;
    if (*list == NULL)
    {
        *list = node;
    }
    else
    {
        node->prev = (*list)->prev;
        (*list)->prev->next = node;
    }
    (*list)->prev = node;
}
/**************************************************************************
** 功能	@brief : 向链表头部增加节点
** 输入	@param : list 链表的首节点地址 node 新链表节点
** 输出	@retval: 增加的节点指针
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadAddFirst(TWLLHead_T **list, TWLLHead_T *node)
{
    node->next = *list;
    if(NULL == *list)
    {
        node->prev = node; 
    }
    else
    {
        node->prev = (*list)->prev;
        (*list)->prev = node;
    }
    *list = node;
}
/**************************************************************************
** 功能	@brief : 删除指定节点
** 输入	@param : list 链表的首节点地址  node 待删除链表节点
** 输出	@retval: 
***************************************************************************/
void FUNCTION_ATTR_ROM Quos_twllHeadDelete(TWLLHead_T **list, TWLLHead_T *node)
{
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == node)
        {
            if (node == *list)
            {
                if (node->next)
                {
                    node->next->prev = (*list)->prev;
                }
                *list = node->next;
            }
            else if (node->next)
            {
                node->next->prev = node->prev;
                node->prev->next = node->next;
            }
            else
            {
                (*list)->prev = node->prev;
                node->prev->next = NULL;
            }
            node->prev = NULL;
            node->next = NULL;
            return;
        }
    }
}
/**************************************************************************
** 功能	@brief : 在指定节点前面插入节点
** 输入	@param : list 链表的首节点地址 referNode参考节点 node 新链表节点
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_twllHeadInsertFront(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node)
{
    Quos_twllHeadDelete(list, node);
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == referNode)
        {
            node->next = referNode;
            node->prev = referNode->prev;

            if (referNode == *list)
            {
                *list = node;
            }
            else
            {
                referNode->prev->next = node;
            }
            referNode->prev = node;
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 在指定节点后面插入节点
** 输入	@param : list 链表的首节点地址 referNode参考节点 node 新链表节点
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_twllHeadInsertBehind(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node)
{
    Quos_twllHeadDelete(list, node);
    TWLLHead_T *tempNode;
    for (tempNode = *list; NULL != tempNode; tempNode = tempNode->next)
    {
        if (tempNode == referNode)
        {
            node->next = referNode->next;
            node->prev = referNode;
            if (referNode->next)
            {
                referNode->next->prev = node;
            }
            referNode->next = node;
            return TRUE;
        }
    }
    return FALSE;
}
/**************************************************************************
** 功能	@brief : 查找链表第N个节点
** 输入	@param : list 链表的首节点地址,nodeId 节点编号第一个节点是0
** 输出	@retval:
***************************************************************************/
TWLLHead_T FUNCTION_ATTR_ROM *Quos_twllHeadFineNodeByNodeId(TWLLHead_T *list, quint32_t nodeId)
{
    TWLLHead_T *tempNode = list;
    while (tempNode && nodeId)
    {
        nodeId--;
        tempNode = tempNode->next;
    }
    return tempNode;
}
/**************************************************************************
** 功能	@brief : 获取指定链表节点数
** 输入	@param : list 链表的首节点地址
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_twllHeadGetNodeCount(TWLLHead_T *list)
{
    quint16_t nodeCnt = 0;
    TWLLHead_T *tempNode = list;
    while (tempNode)
    {
        nodeCnt++;
        tempNode = tempNode->next;
    }
    return nodeCnt;
}
