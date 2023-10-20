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
#ifndef __QHAL_TYPES_H__
#define __QHAL_TYPES_H__
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <strings.h>

#define FUNCTION_ATTR_ROM
#define FUNCTION_ATTR_RAM

#define quint8_t unsigned char
#define qint8_t signed char
#define quint16_t unsigned short
#define qint16_t signed short
#define quint32_t unsigned int
#define qint32_t signed int
#define quint64_t unsigned long long int
#define qint64_t signed long long int
#define pointer_t unsigned long long int
typedef enum{FALSE=0,TRUE=1}qbool;

#define SOCKET_FD_INVALID                   ((pointer_t)-1)
#define QUOS_DNS_HOSTNANE_MAX_LENGHT        (64) /* DNS规定域名不能超过63字符*/
#define QUOS_IP_ADDR_MAX_LEN                (46)

#define HAL_MEMCPY(a,b,l)        memcpy((quint8_t*)(a),(quint8_t*)(b),l)
#define HAL_MEMCMP(a,b,l)        memcmp((quint8_t*)(a),(quint8_t*)(b),l)
#define HAL_MEMSET               memset
#define HAL_MEMMOVE              memmove
#define HAL_SPRINTF              sprintf
#define HAL_SNPRINTF             snprintf
#define HAL_STRCHR(X,Y)          strchr(X,Y)
#define HAL_STRCPY(a,b)          strcpy((char*)a,(char*)b)
#define HAL_STRNCPY(a,b,l)       strncpy((char*)a,(char*)b,l)
#define HAL_STRCMP(a,b)          strcmp((char*)a,(char*)b)
#define HAL_STRNCMP(a,b,l)       strncmp((char*)a,(char*)b,l)
#define HAL_STRSTR(a,b)          strstr((char*)a,(char*)b)
#define HAL_STRLEN(a)            ((NULL==a)?0:strlen((char*)a))
#define HAL_STRNCASECMP(a,b,l)   strncasecmp((char*)a,(char*)b,l)
#define HAL_PRINTF(format,...)   printf(format,##__VA_ARGS__)
#define HAL_SSCANF               sscanf
#define HAL_ATOI				 atoi
#define HAL_STRTOUL              strtoul
#define HAL_STRTOD               strtod
#define HAL_MALLOC(LEN)          malloc(LEN)
#define HAL_FREE(ADDR)           free(ADDR)
#define HAL_STRDUP(X)            strdup(X)
#define HAL_MEMDUP(X,Y)          memdup(X,Y)

#endif
