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
#ifndef __QUOS_TWLL_H__
#define __QUOS_TWLL_H__

#include "Qhal_types.h"

typedef struct __TWLLHead
{
    struct __TWLLHead *prev;
    struct __TWLLHead *next;
}TWLLHead_T;
/* 对节点轮询 */
#define TWLIST_FOR_DATA(LISTHEAD,LISTTEMP,NEXTLIST)    \
    for((LISTTEMP) = (LISTHEAD),(NEXTLIST) = (LISTHEAD)?(LISTHEAD)->next:NULL;   \
        NULL != (LISTTEMP);   \
        (LISTTEMP) = (NEXTLIST),(NEXTLIST) = (NEXTLIST)?(NEXTLIST)->next:NULL) 

void Quos_twllHeadAdd(TWLLHead_T **twList,TWLLHead_T *twNode);
void Quos_twllHeadAddFirst(TWLLHead_T **list, TWLLHead_T *node);
void Quos_twllHeadDelete(TWLLHead_T **twList,TWLLHead_T *twNode);
qbool Quos_twllHeadInsertFront(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node);
qbool Quos_twllHeadInsertBehind(TWLLHead_T **list, TWLLHead_T *referNode, TWLLHead_T *node);
TWLLHead_T *Quos_twllHeadFineNodeByNodeId(TWLLHead_T *list, quint32_t nodeId);
quint32_t Quos_twllHeadGetNodeCount(TWLLHead_T *list);


#endif
