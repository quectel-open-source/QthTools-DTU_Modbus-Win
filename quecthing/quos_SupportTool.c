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
** 功能   @brief   : 工具类API和宏定义
** 硬件   @hardware：任何ANSI-C平台
** 其他   @other   ：
***************************************************************************/
#include "quos_SupportTool.h"
/**************************************************************************
** 功能  @brief : 得到value以exp为底的指数.
** 输入  @param : value > 0 exp > 1
** 输出  @retval: 以exp为底的指数值
***************************************************************************/
quint8_t FUNCTION_ATTR_ROM Quos_convertToExp(quint32_t value, quint32_t exp)
{
	quint8_t count = 0;
	if (exp <= 1)
	{
		return 0;
	}

	while (value)
	{
		value = value / exp;
		count++;
	}
	return count - 1;
}

/**************************************************************************
** 功能	@brief : 将字节流转为字符串
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_hex2Str(quint8_t hex[], quint32_t hexLen, char *retStr, qbool isUpper)
{
	char Hex2strLBuf[] = "0123456789abcdef";
	char Hex2strUBuf[] = "0123456789ABCDEF";
	quint32_t i;
	if (isUpper)
	{
		for (i = 0; i < hexLen; i++)
		{
			retStr[i << 1] = Hex2strUBuf[hex[i] >> 4];
			retStr[(i << 1) + 1] = Hex2strUBuf[hex[i] & 0x0F];
		}
	}
	else
	{
		for (i = 0; i < hexLen; i++)
		{
			retStr[i << 1] = Hex2strLBuf[hex[i] >> 4];
			retStr[(i << 1) + 1] = Hex2strLBuf[hex[i] & 0x0F];
		}
	}
	retStr[i << 1] = 0;
	return hexLen * 2;
}

/**************************************************************************
** 功能	@brief : 将字符串转为字节流
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_str2Hex(void *srcStr, quint8_t retHex[])
{
	quint16_t i;
	char *str = (char *)srcStr;
	quint16_t strLen = HAL_STRLEN(str);
	for (i = 0; i < strLen; i++)
	{
		if (str[i] >= 'A' && str[i] <= 'F')
		{
			if (i % 2)
				retHex[i >> 1] |= (str[i] - 'A' + 10);
			else
				retHex[i >> 1] = (str[i] - 'A' + 10) << 4;
		}
		else if (str[i] >= 'a' && str[i] <= 'f')
		{
			if (i % 2)
				retHex[i >> 1] |= (str[i] - 'a' + 10);
			else
				retHex[i >> 1] = (str[i] - 'a' + 10) << 4;
		}
		else if (str[i] >= '0' && str[i] <= '9')
		{
			if (i % 2)
				retHex[i >> 1] |= (str[i] - '0');
			else
				retHex[i >> 1] = (str[i] - '0') << 4;
		}
		else
		{
			return 0;
		}
	}
	return (i + 1) / 2;
}

/**************************************************************************
** 功能	@brief : crc计算
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_crcCalculate(quint32_t crc, const void *buf, quint32_t len)
{
	quint8_t *dat = (quint8_t *)buf;
	while (len--)
	{
		crc += *dat++;
	}
	return crc;
}
/**************************************************************************
** 功能	@brief : KEY-VALUE提取数据
** 输入	@param : srcStr:key-value键值对源字符串
				 keyword关键字，包括key-value分隔符
				 dstStr输出buffer
				 dstLenMax 输出buffer最大长度，value值超出这长度视为无效
				 endStr key-value结束分隔符,srcStr为多组kv时，endStr必须为非NULL
				 eg:Quos_keyValueExtract("qq:1;ww:2;ee:3","ww:",val,30,";")提取key为ww的val是"2"
** 输出	@retval:
***************************************************************************/
qint32_t FUNCTION_ATTR_ROM Quos_keyValueExtract(char *srcStr, const char *keyword, const char *separator, char **dstStr, const char *endStr)
{
	if (NULL == srcStr || NULL == keyword || NULL == separator || NULL == dstStr)
	{
		return -1;
	}
	char *head = (char *)srcStr;
	while (NULL != (head = HAL_STRSTR(head, keyword)))
	{
		if (HAL_STRNCMP(head + HAL_STRLEN(keyword), separator, HAL_STRLEN(separator)))
		{
			/* do no */
		}
		else if (head != srcStr && NULL == endStr)
		{
			return 0;
		}
		else if (head == srcStr || ((quint32_t)(head - srcStr) >= (quint32_t)HAL_STRLEN(endStr) && 0 == HAL_STRNCMP(&head[0 - HAL_STRLEN(endStr)], endStr, HAL_STRLEN(endStr))))
		{
			break;
		}
		head++;
	}
	if (NULL == head)
	{
		return -1;
	}
	char *tail = NULL;
	head += HAL_STRLEN(keyword) + HAL_STRLEN(separator);
	if (endStr)
	{
		tail = HAL_STRSTR(head, endStr);
	}
	qint32_t valueLen = tail ? (quint32_t)(tail - head) : HAL_STRLEN(head);
	*dstStr = head;
	return valueLen;
}
/**************************************************************************
** 功能	@brief : KEY-VALUE增删改
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_keyValueInsert(char *srcStr, quint32_t maxLen, const char *keyword, const char *separator, const char *value, char *endStr)
{
	if (NULL == srcStr || NULL == keyword || 0 == HAL_STRLEN(keyword) || NULL == separator)
	{
		return FALSE;
	}
	char *head = srcStr;

	/* 查找keyword是否存在 */
	while (NULL != (head = HAL_STRSTR(head, keyword)))
	{
		if (0 != HAL_STRNCMP(head + HAL_STRLEN(keyword), separator, HAL_STRLEN(separator))) /* 并非完整keyword，如何在字段中abcd:1查找keyword为abc， 由于匹配到abc后面并非是：，所以匹配错误 */
		{
			head++;
		}
		else if (head != srcStr && NULL == endStr)
		{
			srcStr[0] = 0;
			head = NULL;
			break;
		}
		else if (head == srcStr || ((quint32_t)(head - srcStr) >= (quint32_t)HAL_STRLEN(endStr) && 0 == HAL_STRNCMP(&head[0 - HAL_STRLEN(endStr)], endStr, HAL_STRLEN(endStr))))
		{
			qint32_t valueLen = HAL_STRLEN(head);
			if (endStr)
			{
				char *tail = HAL_STRSTR(head + HAL_STRLEN(keyword) + HAL_STRLEN(separator), endStr);
				while (tail)
				{
					tail += HAL_STRLEN(endStr);
					valueLen = tail - head;
					if (0 == HAL_STRLEN(tail) || 0 != HAL_STRNCMP(tail, endStr, HAL_STRLEN(endStr)))
					{
						break;
					}
				}
			}
			HAL_MEMMOVE(head, head + valueLen, HAL_STRLEN(head + valueLen) + 1);
		}
		else
		{
			head++;
		}
	}
	if (endStr && HAL_STRLEN(srcStr) >= HAL_STRLEN(endStr) && 0 == HAL_STRNCMP(srcStr + HAL_STRLEN(srcStr) - HAL_STRLEN(endStr), endStr, HAL_STRLEN(endStr)))
	{
		srcStr[HAL_STRLEN(srcStr) - HAL_STRLEN(endStr)] = 0;
	}

	if (value && HAL_STRLEN(value) > 0)
	{
		if (HAL_STRLEN(srcStr) + HAL_STRLEN(keyword) + HAL_STRLEN(separator) + HAL_STRLEN(value) + HAL_STRLEN(endStr) >= maxLen)
		{
			return FALSE;
		}
		HAL_SPRINTF(srcStr + HAL_STRLEN(srcStr), "%s%s%s%s", (HAL_STRLEN(srcStr) > 0 && endStr) ? endStr : "", keyword, separator, value);
	}
	return TRUE;
}
/**************************************************************************
** 功能	@brief : 字符串分解
** 输入	@param : srcStr源字符串，内容将会改变
				 words分解字符串指针
				 maxSize最大支持分解提取个数
				 delim分隔符字符串
** 输出	@retval: 分解提取的个数
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_stringSplit(char *src, quint32_t srcLen, char **words, quint32_t maxSize, const char *delim, qbool keepEmptyParts)
{
	char *start = src;
	quint32_t num = 0;
	if (NULL == delim)
	{
		return 0;
	}
	quint32_t delimLen = HAL_STRLEN(delim);
	while (maxSize > num && src && (quint32_t)(src-start) <srcLen )
	{
		words[num] = src;
		src = HAL_STRSTR(src, delim);
		if (src)
		{
			HAL_MEMSET(src, 0, delimLen);
			src += delimLen;
		}
		if (keepEmptyParts || HAL_STRLEN(words[num]))
		{
			num++;
		}
	}
	return num;
}
/**************************************************************************
** 功能	@brief : 字符串是否正整数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool Quos_strIsUInt(char *src, quint32_t len, quint32_t *value)
{
	quint32_t i = 0;
	if (NULL == src || 0 == len || (len > 1 && '0' == src[0]))
	{
		return FALSE;
	}
	if (value)
	{
		*value = 0;
	}
	for (i = 0; i < len; i++)
	{
		if (src[i] < '0' || src[i] > '9')
		{
			return FALSE;
		}
		else if (value)
		{
			*value = (*value) * 10 + src[i] - '0';
		}
	}
	return TRUE;
}
/**************************************************************************
** 功能	@brief : url解析
** 输入	@param :
** 输出	@retval:
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_urlAnalyze(const char *url, urlAnalyze_t *result)
{
	char *tempUrl;
	quint8_t i;
	HAL_MEMSET(result, 0, sizeof(urlAnalyze_t));
	if (NULL == url || 0 == HAL_STRLEN(url))
	{
		return FALSE;
	}
	if (HAL_STRSTR(url, "s://") || HAL_STRSTR(url, "S://"))
	{
		result->isSecure = TRUE;
	}
	else
	{
		result->isSecure = FALSE;
	}

	tempUrl = HAL_STRSTR(url, "://");
	tempUrl = tempUrl ? (tempUrl + HAL_STRLEN("://")) : (char *)url;

	i = 0;
	while ('\0' != *tempUrl && ':' != *tempUrl && '/' != *tempUrl && i < QUOS_DNS_HOSTNANE_MAX_LENGHT)
	{
		result->hostname[i++] = *tempUrl++;
	}
	if (QUOS_DNS_HOSTNANE_MAX_LENGHT == i || i < 4 || (NULL == HAL_STRCHR(result->hostname, '.') && NULL == HAL_STRCHR(result->hostname, ':')))
	{
		return FALSE;
	}
	result->port = 0;
	if (':' == *tempUrl)
	{
		result->port = HAL_ATOI(tempUrl + 1);
	}
	tempUrl = HAL_STRSTR(tempUrl, "/");
	result->path = tempUrl ? tempUrl + 1 : NULL;
	return TRUE;
}
/**************************************************************************
** 功能	@brief : IP字符串转数值
** 输入	@param :
** 输出	@retval:
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_ip2Int(const char *ipStr)
{
	quint32_t ipInt = 0;
	quint8_t dots = 0;
	quint32_t secValue = 0;
	if (NULL == ipStr || *ipStr < '0' || *ipStr > '9')
	{
		return 0;
	}
	while (ipStr && *ipStr >= '0' && *ipStr <= '9')
	{
		secValue = secValue * 10 + (*ipStr++) - '0';
		if (secValue > 255)
		{
			return 0;
		}
		if (*ipStr == '.')
		{
			ipStr++;
			dots++;
			ipInt = ipInt * 256 + secValue;
			secValue = 0;
		}
		else if (*ipStr == 0 && 3 == dots)
		{
			return ipInt * 256 + secValue;
		}
	}
	return 0;
}

/**************************************************************************
** 功能	@brief : strtoul函数重构
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
quint64_t FUNCTION_ATTR_RAM Quos_strtoul(const char *cp, char **endp, quint32_t base)
{
	unsigned long result = 0, value;
	if (!base)
	{
		base = 10;
		if (*cp == '0')
		{
			base = 8;
			cp++;
			if ((__TO_LOWER(*cp) == 'x') && __IS_XDIGIT(cp[1]))
			{
				cp++;
				base = 16;
			}
		}
	}
	else if (base == 16)
	{
		if (cp[0] == '0' && __TO_LOWER(cp[1]) == 'x')
			cp += 2;
	}
	while (__IS_XDIGIT(*cp) && (value = __IS_DIGIT(*cp) ? *cp - '0' : __TO_LOWER(*cp) - 'a' + 10) < base)
	{
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}
qint64_t FUNCTION_ATTR_RAM Quos_strtol(const char *cp, char **endp, quint32_t base)
{
	if (*cp == '-')
		return -Quos_strtoul(cp + 1, endp, base);
	return Quos_strtoul(cp, endp, base);
}

/**************************************************************************
** 功能	@brief : 数值压缩push到数组
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
quint32_t FUNCTION_ATTR_ROM Quos_intPushArray(quint64_t intValue, quint8_t *array)
{
	quint32_t i;
	quint8_t temp[8];
	_U64_ARRAY01234567(intValue, temp);
	for (i = 0; i < sizeof(temp) - 1; i++)
	{
		if (0 != temp[i])
		{
			break;
		}
	}
	HAL_MEMCPY(array, &temp[i], sizeof(temp) - i);
	return sizeof(temp) - i;
}
/**************************************************************************
 ** 功能	@brief : 字符串若存在双引号则去掉,strVal内容将会改变
 ** 输入	@param : 
 ** 输出	@retval: 
 ***************************************************************************/
char FUNCTION_ATTR_ROM *Quos_stringRemoveMarks(char *strVal)
{
	quint32_t len = HAL_STRLEN(strVal);
	if ('"' == strVal[0] && '"' == strVal[len - 1])
	{
		quint32_t i;
		for (i = 0; i < len - 2; i++)
		{
			strVal[i] = strVal[i + 1];
		}
		strVal[len - 2] = 0;
	}
	return strVal;
}

/**************************************************************************
** 功能	@brief : 判断是否为整数
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
qbool FUNCTION_ATTR_ROM Quos_numIsDouble(double value)
{
	if(value < 0)
	{
		value = 0 - value;
	}
	if(value == (quint64_t)value)
	{
		return FALSE;
	}
	return TRUE;
}
