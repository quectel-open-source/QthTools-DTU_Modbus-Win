/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */
/* disable warnings about old C89 functions in MSVC */
#include "quos_cjson.h"
#include <math.h>
#include "quos_SupportTool.h"
/* define isnan and isinf for ANSI C, if in C99 or above, isnan and isinf has been defined in math.h */
#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef NAN
#define NAN 0.0/0.0
#endif
/* project version */
#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 14

typedef struct {
    const unsigned char *json;
    quint32_t position;
} error;
static error global_error = { NULL, 0 };

const char * cJSON_GetErrorPtr(void)
{
    return (const char*) (global_error.json + global_error.position);
}

char * cJSON_GetStringValue(const cJSON * const item)
{
    if (!cJSON_IsString(item)) 
    {
        return NULL;
    }

    return item->valuestring;
}

double cJSON_GetNumberValue(const cJSON * const item)
{
    if (!cJSON_IsNumber(item)) 
    {
        return (double) NAN;
    }

    return item->valuedouble;
}

/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (CJSON_VERSION_MAJOR != 1) || (CJSON_VERSION_MINOR != 7) || (CJSON_VERSION_PATCH != 14)
    #error cJSON.h and cJSON.c have different versions. Make sure that both have the same.
#endif

const char* cJSON_Version(void)
{
    static char version[15];
    HAL_SPRINTF(version, "%i.%i.%i", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);

    return version;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int case_insensitive_strcmp(const unsigned char *string1, const unsigned char *string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for(; __TO_LOWER(*string1) == __TO_LOWER(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return __TO_LOWER(*string1) - __TO_LOWER(*string2);
}

/* HAL_STRLEN of character literals resolved at compile time */
#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

/* Internal constructor. */
static cJSON *cJSON_New_Item(void)
{
    cJSON* node = HAL_MALLOC(sizeof(cJSON));
    if (node)
    {
        HAL_MEMSET(node, '\0', sizeof(cJSON));
    }

    return node;
}

/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *item)
{
    cJSON *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & QUOS_cJSON_IsReference) && (item->child != NULL))
        {
            cJSON_Delete(item->child);
        }
        if (!(item->type & QUOS_cJSON_IsReference) && (item->valuestring != NULL))
        {
            HAL_FREE(item->valuestring);
        }
        if (!(item->type & QUOS_cJSON_StringIsConst) && (item->string != NULL))
        {
            HAL_FREE(item->string);
        }
        HAL_FREE(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (unsigned char) lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char *content;
    quint32_t length;
    quint32_t offset;
    quint32_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static cJSON_bool parse_number(cJSON * const item, parse_buffer * const input_buffer)
{
    double number = 0;
    unsigned char *after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    quint32_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return FALSE;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_c_string[i] = buffer_at_offset(input_buffer)[i];
                break;

            case '.':
                number_c_string[i] = decimal_point;
                break;

            default:
                goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = HAL_STRTOD((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        return FALSE; /* parse_error */
    }

    item->valuedouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
    {
        item->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        item->valueint = INT_MIN;
    }
    else
    {
        item->valueint = (int)number;
    }

    item->type = QUOS_cJSON_Number;

    input_buffer->offset += (quint32_t)(after_end - number_c_string);
    return TRUE;
}

/* don't ask me, but the original cJSON_SetNumberValue returns an integer or double */
double cJSON_SetNumberHelper(cJSON *object, double number)
{
    if (number >= INT_MAX)
    {
        object->valueint = INT_MAX;
    }
    else if (number <= (double)INT_MIN)
    {
        object->valueint = INT_MIN;
    }
    else
    {
        object->valueint = (int)number;
    }

    return object->valuedouble = number;
}

char* cJSON_SetValuestring(cJSON *object, const char *valuestring)
{
    char *copy = NULL;
    /* if object's type is not QUOS_cJSON_String or is QUOS_cJSON_IsReference, it should not set valuestring */
    if (!(object->type & QUOS_cJSON_String) || (object->type & QUOS_cJSON_IsReference))
    {
        return NULL;
    }
    if (HAL_STRLEN(valuestring) <= HAL_STRLEN(object->valuestring))
    {
        HAL_STRCPY(object->valuestring, valuestring);
        return object->valuestring;
    }
    copy = HAL_STRDUP(valuestring);
    if (copy == NULL)
    {
        return NULL;
    }
    if (object->valuestring != NULL)
    {
        HAL_FREE(object->valuestring);
    }
    object->valuestring = copy;

    return copy;
}

typedef struct
{
    unsigned char *buffer;
    quint32_t length;
    quint32_t offset;
    quint32_t depth; /* current nesting depth (for formatted printing) */
    cJSON_bool noalloc;
    cJSON_bool format; /* is this print a formatted print */
} printbuffer;

/* realloc printbuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(printbuffer * const p, quint32_t needed)
{
    unsigned char *newbuffer = NULL;
    quint32_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL))
    {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX)
    {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2))
    {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX)
        {
            newsize = INT_MAX;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

        /* otherwise reallocate manually */
        newbuffer = HAL_MALLOC(newsize);
        if (!newbuffer)
        {
            HAL_FREE(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
        if (newbuffer)
        {
            HAL_MEMCPY(newbuffer, p->buffer, p->offset + 1);
        }
        HAL_FREE(p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
static void update_offset(printbuffer * const buffer)
{
    const unsigned char *buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += HAL_STRLEN((const char*)buffer_pointer);
}

/* securely comparison of floating-point variables */
static cJSON_bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Render the number nicely from the given item into a string. */
static cJSON_bool print_number(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    quint32_t i = 0;
    unsigned char number_buffer[26] = {0}; /* temporary buffer to print the number into */
    unsigned char decimal_point = get_decimal_point();
    double test = 0.0;

    if (output_buffer == NULL)
    {
        return FALSE;
    }

    /* This checks for NaN and Infinity */
    if (isnan(d) || isinf(d))
    {
        length = HAL_SPRINTF((char*)number_buffer, "null");
    }
    else
    {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = HAL_SPRINTF((char*)number_buffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((HAL_SSCANF((char*)number_buffer, "%lg", &test) != 1) || !compare_double((double)test, d))
        {
            /* If not, print with 17 decimal places of precision */
            length = HAL_SPRINTF((char*)number_buffer, "%1.17g", d);
        }
    }

    /* HAL_SPRINTF failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return FALSE;
    }

    /* reserve appropriate space in the output */
    output_pointer = ensure(output_buffer, (quint32_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        return FALSE;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((quint32_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (quint32_t)length;

    return TRUE;
}

/* parse 4 digit hexadecimal number */
static unsigned parse_hex4(const unsigned char * const input)
{
    unsigned int h = 0;
    quint32_t i = 0;

    for (i = 0; i < 4; i++)
    {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else /* invalid */
        {
            return 0;
        }

        if (i < 3)
        {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16_literal_to_utf8(const unsigned char * const input_pointer, const unsigned char * const input_end, unsigned char **output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char *first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    first_code = parse_hex4(first_sequence + 2);

    /* check that the code is valid */
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char *second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12; /* \uXXXX\uXXXX */

        if ((input_end - second_sequence) < 6)
        {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        second_code = parse_hex4(second_sequence + 2);
        /* check that the code is valid */
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            /* invalid second half of the surrogate pair */
            goto fail;
        }


        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; /* \uXXXX */
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80)
    {
        /* normal ascii, encoding 0xxxxxxx */
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8_length = 2;
        first_byte_mark = 0xC0; /* 11000000 */
    }
    else if (codepoint < 0x10000)
    {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8_length = 3;
        first_byte_mark = 0xE0; /* 11100000 */
    }
    else if (codepoint <= 0x10FFFF)
    {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8_length = 4;
        first_byte_mark = 0xF0; /* 11110000 */
    }
    else
    {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        /* 10xxxxxx */
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static cJSON_bool parse_string(cJSON * const item, parse_buffer * const input_buffer)
{
    const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char *output_pointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        quint32_t allocation_length = 0;
        quint32_t skipped_bytes = 0;
        while (((quint32_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            /* is escape sequence */
            if (input_end[0] == '\\')
            {
                if ((quint32_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((quint32_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocation_length = (quint32_t) (input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = HAL_MALLOC(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail; /* allocation failure */
        }
    }

    output_pointer = output;
    /* loop through the string literal */
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        /* escape sequence */
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    /* zero terminate the output */
    *output_pointer = '\0';

    item->type = QUOS_cJSON_String;
    item->valuestring = (char*)output;

    input_buffer->offset = (quint32_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    return TRUE;

fail:
    if (output != NULL)
    {
        HAL_FREE(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (quint32_t)(input_pointer - input_buffer->content);
    }

    return FALSE;
}

/* Render the cstring provided to an escaped version that can be printed. */
static cJSON_bool print_string_ptr(const unsigned char * const input, printbuffer * const output_buffer)
{
    const unsigned char *input_pointer = NULL;
    unsigned char *output = NULL;
    unsigned char *output_pointer = NULL;
    quint32_t output_length = 0;
    /* numbers of additional characters needed for escaping */
    quint32_t escape_characters = 0;

    if (output_buffer == NULL)
    {
        return FALSE;
    }

    /* empty string */
    if (input == NULL)
    {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            return FALSE;
        }
        HAL_STRCPY((char*)output, "\"\"");

        return TRUE;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escape_characters++;
                break;
            default:
                if (*input_pointer < 32)
                {
                    /* UTF-16 escape sequence uXXXX */
                    escape_characters += 5;
                }
                break;
        }
    }
    output_length = (quint32_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        return FALSE;
    }

    /* no characters have to be escaped */
    if (escape_characters == 0)
    {
        output[0] = '\"';
        HAL_MEMCPY(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return TRUE;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    /* copy the string */
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            /* normal character, copy */
            *output_pointer = *input_pointer;
        }
        else
        {
            /* character needs to be escaped */
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
                case '\\':
                    *output_pointer = '\\';
                    break;
                case '\"':
                    *output_pointer = '\"';
                    break;
                case '\b':
                    *output_pointer = 'b';
                    break;
                case '\f':
                    *output_pointer = 'f';
                    break;
                case '\n':
                    *output_pointer = 'n';
                    break;
                case '\r':
                    *output_pointer = 'r';
                    break;
                case '\t':
                    *output_pointer = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    HAL_SPRINTF((char*)output_pointer, "u%04x", *input_pointer);
                    output_pointer += 4;
                    break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return TRUE;
}

/* Invoke print_string_ptr (which is useful) on an item. */
static cJSON_bool print_string(const cJSON * const item, printbuffer * const p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

/* Predeclare these prototypes. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer);
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer);
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer);

/* Utility to jump whitespace and cr/lf */
static parse_buffer *buffer_skip_whitespace(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
       buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static parse_buffer *skip_utf8_bom(parse_buffer * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (HAL_STRNCMP((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

cJSON * cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    quint32_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    /* Adding null character size due to require_null_terminated. */
    buffer_length = HAL_STRLEN(value) + sizeof("");

    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

/* Parse an object - create a new root, and populate. */
cJSON * cJSON_ParseWithLengthOpts(const char *value, quint32_t buffer_length, const char **return_parse_end, cJSON_bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0 };
    cJSON *item = NULL;

    /* reset error position */
    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length; 
    buffer.offset = 0;

    item = cJSON_New_Item();
    if (item == NULL) /* memory fail */
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        cJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}

/* Default options for cJSON_Parse */
cJSON * cJSON_Parse(const char *value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

cJSON * cJSON_ParseWithLength(const char *value, quint32_t buffer_length)
{
    return cJSON_ParseWithLengthOpts(value, buffer_length, 0, 0);
}

#define cjson_min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char *print(const cJSON * const item, cJSON_bool format)
{
    static const quint32_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char *printed = NULL;

    HAL_MEMSET(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = HAL_MALLOC(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    /* print the value */
    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

        printed = HAL_MALLOC(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        HAL_MEMCPY(printed, buffer->buffer, cjson_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0'; /* just to be sure */

        /* free the buffer */
        HAL_FREE(buffer->buffer);

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        HAL_FREE(buffer->buffer);
    }

    if (printed != NULL)
    {
        HAL_FREE(printed);
    }

    return NULL;
}

/* Render a cJSON item/entity/structure to text. */
char * cJSON_Print(const cJSON *item)
{
    return (char*)print(item, TRUE);
}

char * cJSON_PrintUnformatted(const cJSON *item)
{
    return (char*)print(item, FALSE);
}

char * cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0};

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = HAL_MALLOC((quint32_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (quint32_t)prebuffer;
    p.offset = 0;
    p.noalloc = FALSE;
    p.format = fmt;

    if (!print_value(item, &p))
    {
        HAL_FREE(p.buffer);
        return NULL;
    }

    return (char*)p.buffer;
}

cJSON_bool cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0};

    if ((length < 0) || (buffer == NULL))
    {
        return FALSE;
    }

    p.buffer = (unsigned char*)buffer;
    p.length = (quint32_t)length;
    p.offset = 0;
    p.noalloc = TRUE;
    p.format = format;

    return print_value(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static cJSON_bool parse_value(cJSON * const item, parse_buffer * const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return FALSE; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (can_read(input_buffer, 4) && (HAL_STRNCASECMP((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = QUOS_cJSON_NULL;
        input_buffer->offset += 4;
        return TRUE;
    }
    /* FALSE */
    if (can_read(input_buffer, 5) && (HAL_STRNCASECMP((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = QUOS_cJSON_False;
        input_buffer->offset += 5;
        return TRUE;
    }
    /* TRUE */
    if (can_read(input_buffer, 4) && (HAL_STRNCASECMP((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = QUOS_cJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return TRUE;
    }
    /* string */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_string(item, input_buffer);
    }
    /* number */
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_number(item, input_buffer);
    }
    /* array */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    /* object */
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }

    return FALSE;
}

/* Render a value to text. */
static cJSON_bool print_value(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return FALSE;
    }

    switch ((item->type) & 0xFF)
    {
        case QUOS_cJSON_NULL:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return FALSE;
            }
            HAL_STRCPY((char*)output, "null");
            return TRUE;

        case QUOS_cJSON_False:
            output = ensure(output_buffer, 6);
            if (output == NULL)
            {
                return FALSE;
            }
            HAL_STRCPY((char*)output, "false");
            return TRUE;

        case QUOS_cJSON_True:
            output = ensure(output_buffer, 5);
            if (output == NULL)
            {
                return FALSE;
            }
            HAL_STRCPY((char*)output, "true");
            return TRUE;

        case QUOS_cJSON_Number:
            return print_number(item, output_buffer);

        case QUOS_cJSON_Raw:
        {
            quint32_t raw_length = 0;
            if (item->valuestring == NULL)
            {
                return FALSE;
            }

            raw_length = HAL_STRLEN(item->valuestring) + sizeof("");
            output = ensure(output_buffer, raw_length);
            if (output == NULL)
            {
                return FALSE;
            }
            HAL_MEMCPY(output, item->valuestring, raw_length);
            return TRUE;
        }

        case QUOS_cJSON_String:
            return print_string(item, output_buffer);

        case QUOS_cJSON_Array:
            return print_array(item, output_buffer);

        case QUOS_cJSON_Object:
            return print_object(item, output_buffer);

        default:
            return FALSE;
    }
}

/* Build an array from input text. */
static cJSON_bool parse_array(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* head of the linked list */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= QUOS_CJSON_NESTING_LIMIT)
    {
        return FALSE; /* to deeply nested */
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        /* not an array */
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item();
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse next value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail; /* expected end of array */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = QUOS_cJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return TRUE;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return FALSE;
}

/* Render an array to text */
static cJSON_bool print_array(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    quint32_t length = 0;
    cJSON *current_element = item->child;

    if (output_buffer == NULL)
    {
        return FALSE;
    }

    /* Compose the output array. */
    /* opening square bracket */
    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        return FALSE;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    while (current_element != NULL)
    {
        if (!print_value(current_element, output_buffer))
        {
            return FALSE;
        }
        update_offset(output_buffer);
        if (current_element->next)
        {
            length = (quint32_t) (output_buffer->format ? 2 : 1);
            output_pointer = ensure(output_buffer, length + 1);
            if (output_pointer == NULL)
            {
                return FALSE;
            }
            *output_pointer++ = ',';
            if(output_buffer->format)
            {
                *output_pointer++ = ' ';
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        return FALSE;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return TRUE;
}

/* Build an object from the text. */
static cJSON_bool parse_object(cJSON * const item, parse_buffer * const input_buffer)
{
    cJSON *head = NULL; /* linked list head */
    cJSON *current_item = NULL;

    if (input_buffer->depth >= QUOS_CJSON_NESTING_LIMIT)
    {
        return FALSE; /* to deeply nested */
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail; /* not an object */
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    input_buffer->offset--;
    /* loop through the comma separated array elements */
    do
    {
        /* allocate next item */
        cJSON *new_item = cJSON_New_Item();
        if (new_item == NULL)
        {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL)
        {
            /* start the linked list */
            current_item = head = new_item;
        }
        else
        {
            /* add to the end and advance */
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        /* parse the name of the child */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer))
        {
            goto fail; /* failed to parse name */
        }
        buffer_skip_whitespace(input_buffer);

        /* swap valuestring and string, because we parsed the name */
        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail; /* failed to parse value */
        }
        buffer_skip_whitespace(input_buffer);
    }
    while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail; /* expected end of object */
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = QUOS_cJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return TRUE;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return FALSE;
}

/* Render an object to text. */
static cJSON_bool print_object(const cJSON * const item, printbuffer * const output_buffer)
{
    unsigned char *output_pointer = NULL;
    quint32_t length = 0;
    cJSON *current_item = item->child;

    if (output_buffer == NULL)
    {
        return FALSE;
    }

    /* Compose the output: */
    length = (quint32_t) (output_buffer->format ? 2 : 1); /* fmt: {\n */
    output_pointer = ensure(output_buffer, length + 1);
    if (output_pointer == NULL)
    {
        return FALSE;
    }

    *output_pointer++ = '{';
    output_buffer->depth++;
    if (output_buffer->format)
    {
        *output_pointer++ = '\n';
    }
    output_buffer->offset += length;

    while (current_item)
    {
        if (output_buffer->format)
        {
            quint32_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL)
            {
                return FALSE;
            }
            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        /* print key */
        if (!print_string_ptr((unsigned char*)current_item->string, output_buffer))
        {
            return FALSE;
        }
        update_offset(output_buffer);

        length = (quint32_t) (output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL)
        {
            return FALSE;
        }
        *output_pointer++ = ':';
        if (output_buffer->format)
        {
            *output_pointer++ = '\t';
        }
        output_buffer->offset += length;

        /* print value */
        if (!print_value(current_item, output_buffer))
        {
            return FALSE;
        }
        update_offset(output_buffer);

        /* print comma if not last */
        length = ((quint32_t)(output_buffer->format ? 1 : 0) + (quint32_t)(current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return FALSE;
        }
        if (current_item->next)
        {
            *output_pointer++ = ',';
        }

        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL)
    {
        return FALSE;
    }
    if (output_buffer->format)
    {
        quint32_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++)
        {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return TRUE;
}

/* Get Array size/item / object item. */
int cJSON_GetArraySize(const cJSON *array)
{
    cJSON *child = NULL;
    quint32_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while(child != NULL)
    {
        size++;
        child = child->next;
    }

    /* FIXME: Can overflow here. Cannot be fixed without breaking the API */

    return (int)size;
}

static cJSON* get_array_item(const cJSON *array, quint32_t index)
{
    cJSON *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

cJSON * cJSON_GetArrayItem(const cJSON *array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return get_array_item(array, (quint32_t)index);
}

static cJSON *get_object_item(const cJSON * const object, const char * const name, const cJSON_bool case_sensitive)
{
    cJSON *current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (HAL_STRCMP(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL)) {
        return NULL;
    }

    return current_element;
}

cJSON * cJSON_GetObjectItem(const cJSON * const object, const char * const string)
{
    return get_object_item(object, string, FALSE);
}

cJSON * cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
    return get_object_item(object, string, TRUE);
}

cJSON_bool cJSON_HasObjectItem(const cJSON *object, const char *string)
{
    return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(cJSON *prev, cJSON *item)
{
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static cJSON *create_reference(const cJSON *item)
{
    cJSON *reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = cJSON_New_Item();
    if (reference == NULL)
    {
        return NULL;
    }

    HAL_MEMCPY(reference, item, sizeof(cJSON));
    reference->string = NULL;
    reference->type |= QUOS_cJSON_IsReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static cJSON_bool add_item_to_array(cJSON *array, cJSON *item)
{
    cJSON *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item))
    {
        return FALSE;
    }

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL)
    {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    }
    else
    {
        /* append to the end */
        if (child->prev)
        {
            suffix_object(child->prev, item);
            array->child->prev = item;
        }
    }

    return TRUE;
}

/* Add item to array/object. */
cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    return add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
/* helper function to cast away const */
static void* cast_away_const(const void* string)
{
    return (void*)string;
}
#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
    #pragma GCC diagnostic pop
#endif


static cJSON_bool add_item_to_object(cJSON * const object, const char * const string, cJSON * const item, const cJSON_bool constant_key)
{
    char *new_key = NULL;
    int new_type = QUOS_cJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
    {
        return FALSE;
    }

    if (constant_key)
    {
        new_key = (char*)cast_away_const(string);
        new_type = item->type | QUOS_cJSON_StringIsConst;
    }
    else
    {
        new_key = HAL_STRDUP(string);
        if (new_key == NULL)
        {
            return FALSE;
        }

        new_type = item->type & ~QUOS_cJSON_StringIsConst;
    }

    if (!(item->type & QUOS_cJSON_StringIsConst) && (item->string != NULL))
    {
        HAL_FREE(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    return add_item_to_object(object, string, item, FALSE);
}

/* Add an item to an object with constant string as key */
cJSON_bool cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item)
{
    return add_item_to_object(object, string, item, TRUE);
}

cJSON_bool cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)
{
    if (array == NULL)
    {
        return FALSE;
    }

    return add_item_to_array(array, create_reference(item));
}

cJSON_bool cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item)
{
    if ((object == NULL) || (string == NULL))
    {
        return FALSE;
    }

    return add_item_to_object(object, string, create_reference(item), FALSE);
}

cJSON* cJSON_AddNullToObject(cJSON * const object, const char * const name)
{
    cJSON *null = cJSON_CreateNull();
    if (add_item_to_object(object, name, null, FALSE))
    {
        return null;
    }

    cJSON_Delete(null);
    return NULL;
}

cJSON* cJSON_AddTrueToObject(cJSON * const object, const char * const name)
{
    cJSON *true_item = cJSON_CreateTrue();
    if (add_item_to_object(object, name, true_item, FALSE))
    {
        return true_item;
    }

    cJSON_Delete(true_item);
    return NULL;
}

cJSON* cJSON_AddFalseToObject(cJSON * const object, const char * const name)
{
    cJSON *false_item = cJSON_CreateFalse();
    if (add_item_to_object(object, name, false_item, FALSE))
    {
        return false_item;
    }

    cJSON_Delete(false_item);
    return NULL;
}

cJSON* cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean)
{
    cJSON *bool_item = cJSON_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, FALSE))
    {
        return bool_item;
    }

    cJSON_Delete(bool_item);
    return NULL;
}

cJSON* cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number)
{
    cJSON *number_item = cJSON_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, FALSE))
    {
        return number_item;
    }

    cJSON_Delete(number_item);
    return NULL;
}

cJSON* cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string)
{
    cJSON *string_item = cJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, FALSE))
    {
        return string_item;
    }

    cJSON_Delete(string_item);
    return NULL;
}

cJSON* cJSON_AddRawToObject(cJSON * const object, const char * const name, const char * const raw)
{
    cJSON *raw_item = cJSON_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, FALSE))
    {
        return raw_item;
    }

    cJSON_Delete(raw_item);
    return NULL;
}

cJSON* cJSON_AddObjectToObject(cJSON * const object, const char * const name)
{
    cJSON *object_item = cJSON_CreateObject();
    if (add_item_to_object(object, name, object_item, FALSE))
    {
        return object_item;
    }

    cJSON_Delete(object_item);
    return NULL;
}

cJSON* cJSON_AddArrayToObject(cJSON * const object, const char * const name)
{
    cJSON *array = cJSON_CreateArray();
    if (add_item_to_object(object, name, array, FALSE))
    {
        return array;
    }

    cJSON_Delete(array);
    return NULL;
}

cJSON * cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item)
{
    if ((parent == NULL) || (item == NULL))
    {
        return NULL;
    }

    if (item != parent->child)
    {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        /* first element */
        parent->child = item->next;
    }
    else if (item->next == NULL)
    {
        /* last element */
        parent->child->prev = item->prev;
    }

    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

cJSON * cJSON_DetachItemFromArray(cJSON *array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return cJSON_DetachItemViaPointer(array, get_array_item(array, (quint32_t)which));
}

void cJSON_DeleteItemFromArray(cJSON *array, int which)
{
    cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

cJSON * cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
    cJSON *to_detach = cJSON_GetObjectItem(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

cJSON * cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
    cJSON *to_detach = cJSON_GetObjectItemCaseSensitive(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

void cJSON_DeleteItemFromObject(cJSON *object, const char *string)
{
    cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

void cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
    cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
cJSON_bool cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem)
{
    cJSON *after_inserted = NULL;

    if (which < 0)
    {
        return FALSE;
    }

    after_inserted = get_array_item(array, (quint32_t)which);
    if (after_inserted == NULL)
    {
        return add_item_to_array(array, newitem);
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    return TRUE;
}

cJSON_bool cJSON_ReplaceItemViaPointer(cJSON * const parent, cJSON * const item, cJSON * replacement)
{
    if ((parent == NULL) || (replacement == NULL) || (item == NULL))
    {
        return FALSE;
    }

    if (replacement == item)
    {
        return TRUE;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (parent->child == item)
    {
        if (parent->child->prev == parent->child)
        {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    }
    else
    {   /*
         * To find the last item in array quickly, we use prev in array.
         * We can't modify the last item's next pointer where this item was the parent's child
         */
        if (replacement->prev != NULL)
        {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL)
        {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    cJSON_Delete(item);

    return TRUE;
}

cJSON_bool cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem)
{
    if (which < 0)
    {
        return FALSE;
    }

    return cJSON_ReplaceItemViaPointer(array, get_array_item(array, (quint32_t)which), newitem);
}

static cJSON_bool replace_item_in_object(cJSON *object, const char *string, cJSON *replacement, cJSON_bool case_sensitive)
{
    if ((replacement == NULL) || (string == NULL))
    {
        return FALSE;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & QUOS_cJSON_StringIsConst) && (replacement->string != NULL))
    {
        HAL_FREE(replacement->string);
    }
    replacement->string = HAL_STRDUP(string);
    replacement->type &= ~QUOS_cJSON_StringIsConst;

    return cJSON_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);
}

cJSON_bool cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem)
{
    return replace_item_in_object(object, string, newitem, FALSE);
}

cJSON_bool cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object, const char *string, cJSON *newitem)
{
    return replace_item_in_object(object, string, newitem, TRUE);
}

/* Create basic types: */
cJSON * cJSON_CreateNull(void)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_NULL;
    }

    return item;
}

cJSON * cJSON_CreateTrue(void)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_True;
    }

    return item;
}

cJSON * cJSON_CreateFalse(void)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_False;
    }

    return item;
}

cJSON * cJSON_CreateBool(cJSON_bool boolean)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = boolean ? QUOS_cJSON_True : QUOS_cJSON_False;
    }

    return item;
}

cJSON * cJSON_CreateNumber(double num)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_Number;
        item->valuedouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX)
        {
            item->valueint = INT_MAX;
        }
        else if (num <= (double)INT_MIN)
        {
            item->valueint = INT_MIN;
        }
        else
        {
            item->valueint = (int)num;
        }
    }

    return item;
}

cJSON * cJSON_CreateString(const char *string)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_String;
        item->valuestring = HAL_STRDUP(string);
        if(!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

cJSON * cJSON_CreateStringReference(const char *string)
{
    cJSON *item = cJSON_New_Item();
    if (item != NULL)
    {
        item->type = QUOS_cJSON_String | QUOS_cJSON_IsReference;
        item->valuestring = (char*)cast_away_const(string);
    }

    return item;
}

cJSON * cJSON_CreateObjectReference(const cJSON *child)
{
    cJSON *item = cJSON_New_Item();
    if (item != NULL) {
        item->type = QUOS_cJSON_Object | QUOS_cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

cJSON * cJSON_CreateArrayReference(const cJSON *child)
{
    cJSON *item = cJSON_New_Item();
    if (item != NULL) {
        item->type = QUOS_cJSON_Array | QUOS_cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

cJSON * cJSON_CreateRaw(const char *raw)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type = QUOS_cJSON_Raw;
        item->valuestring = HAL_STRDUP(raw);
        if(!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

cJSON * cJSON_CreateArray(void)
{
    cJSON *item = cJSON_New_Item();
    if(item)
    {
        item->type=QUOS_cJSON_Array;
    }

    return item;
}

cJSON * cJSON_CreateObject(void)
{
    cJSON *item = cJSON_New_Item();
    if (item)
    {
        item->type = QUOS_cJSON_Object;
    }

    return item;
}

/* Create Arrays: */
cJSON * cJSON_CreateIntArray(const int *numbers, int count)
{
    quint32_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();
    for(i = 0; a && (i < (quint32_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }
    a->child->prev = n;

    return a;
}

cJSON * cJSON_CreateFloatArray(const float *numbers, int count)
{
    quint32_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for(i = 0; a && (i < (quint32_t)count); i++)
    {
        n = cJSON_CreateNumber((double)numbers[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }
    a->child->prev = n;

    return a;
}

cJSON * cJSON_CreateDoubleArray(const double *numbers, int count)
{
    quint32_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for(i = 0;a && (i < (quint32_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }
    a->child->prev = n;

    return a;
}

cJSON * cJSON_CreateStringArray(const char *const *strings, int count)
{
    quint32_t i = 0;
    cJSON *n = NULL;
    cJSON *p = NULL;
    cJSON *a = NULL;

    if ((count < 0) || (strings == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for (i = 0; a && (i < (quint32_t)count); i++)
    {
        n = cJSON_CreateString(strings[i]);
        if(!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if(!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p,n);
        }
        p = n;
    }
    a->child->prev = n;

    return a;
}

/* Duplication */
cJSON * cJSON_Duplicate(const cJSON *item, cJSON_bool recurse)
{
    cJSON *newitem = NULL;
    cJSON *child = NULL;
    cJSON *next = NULL;
    cJSON *newchild = NULL;

    /* Bail on bad ptr */
    if (!item)
    {
        goto fail;
    }
    /* Create new item */
    newitem = cJSON_New_Item();
    if (!newitem)
    {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~QUOS_cJSON_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = HAL_STRDUP(item->valuestring);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type&QUOS_cJSON_StringIsConst) ? item->string : HAL_STRDUP(item->string);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse)
    {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL)
    {
        newchild = cJSON_Duplicate(child, TRUE); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }
    if (newitem && newitem->child)
    {
        newitem->child->prev = newchild;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        cJSON_Delete(newitem);
    }

    return NULL;
}

static void skip_oneline_comment(char **input)
{
    *input += static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if ((*input)[0] == '\n') {
            *input += static_strlen("\n");
            return;
        }
    }
}

static void skip_multiline_comment(char **input)
{
    *input += static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if (((*input)[0] == '*') && ((*input)[1] == '/'))
        {
            *input += static_strlen("*/");
            return;
        }
    }
}

static void minify_string(char **input, char **output) {
    (*output)[0] = (*input)[0];
    *input += static_strlen("\"");
    *output += static_strlen("\"");


    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output)) {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"') {
            (*output)[0] = '\"';
            *input += static_strlen("\"");
            *output += static_strlen("\"");
            return;
        } else if (((*input)[0] == '\\') && ((*input)[1] == '\"')) {
            (*output)[1] = (*input)[1];
            *input += static_strlen("\"");
            *output += static_strlen("\"");
        }
    }
}

void cJSON_Minify(char *json)
{
    char *into = json;

    if (json == NULL)
    {
        return;
    }

    while (json[0] != '\0')
    {
        switch (json[0])
        {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                json++;
                break;

            case '/':
                if (json[1] == '/')
                {
                    skip_oneline_comment(&json);
                }
                else if (json[1] == '*')
                {
                    skip_multiline_comment(&json);
                } else {
                    json++;
                }
                break;

            case '\"':
                minify_string(&json, (char**)&into);
                break;

            default:
                into[0] = json[0];
                json++;
                into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

cJSON_bool cJSON_IsInvalid(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_Invalid;
}

cJSON_bool cJSON_IsFalse(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_False;
}

cJSON_bool cJSON_IsTrue(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xff) == QUOS_cJSON_True;
}


cJSON_bool cJSON_IsBool(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & (QUOS_cJSON_True | QUOS_cJSON_False)) != 0;
}
cJSON_bool cJSON_IsNull(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_NULL;
}

cJSON_bool cJSON_IsNumber(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_Number;
}

cJSON_bool cJSON_IsString(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_String;
}

cJSON_bool cJSON_IsArray(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_Array;
}

cJSON_bool cJSON_IsObject(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_Object;
}

cJSON_bool cJSON_IsRaw(const cJSON * const item)
{
    if (item == NULL)
    {
        return FALSE;
    }

    return (item->type & 0xFF) == QUOS_cJSON_Raw;
}

cJSON_bool cJSON_Compare(const cJSON * const a, const cJSON * const b, const cJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || cJSON_IsInvalid(a))
    {
        return FALSE;
    }

    /* check if type is valid */
    switch (a->type & 0xFF)
    {
        case QUOS_cJSON_False:
        case QUOS_cJSON_True:
        case QUOS_cJSON_NULL:
        case QUOS_cJSON_Number:
        case QUOS_cJSON_String:
        case QUOS_cJSON_Raw:
        case QUOS_cJSON_Array:
        case QUOS_cJSON_Object:
            break;

        default:
            return FALSE;
    }

    /* identical objects are equal */
    if (a == b)
    {
        return TRUE;
    }

    switch (a->type & 0xFF)
    {
        /* in these cases and equal type is enough */
        case QUOS_cJSON_False:
        case QUOS_cJSON_True:
        case QUOS_cJSON_NULL:
            return TRUE;

        case QUOS_cJSON_Number:
            if (compare_double(a->valuedouble, b->valuedouble))
            {
                return TRUE;
            }
            return FALSE;

        case QUOS_cJSON_String:
        case QUOS_cJSON_Raw:
            if ((a->valuestring == NULL) || (b->valuestring == NULL))
            {
                return FALSE;
            }
            if (HAL_STRCMP(a->valuestring, b->valuestring) == 0)
            {
                return TRUE;
            }

            return FALSE;

        case QUOS_cJSON_Array:
        {
            cJSON *a_element = a->child;
            cJSON *b_element = b->child;

            for (; (a_element != NULL) && (b_element != NULL);)
            {
                if (!cJSON_Compare(a_element, b_element, case_sensitive))
                {
                    return FALSE;
                }

                a_element = a_element->next;
                b_element = b_element->next;
            }

            /* one of the arrays is longer than the other */
            if (a_element != b_element) {
                return FALSE;
            }

            return TRUE;
        }

        case QUOS_cJSON_Object:
        {
            cJSON *a_element = NULL;
            cJSON *b_element = NULL;
            QUOS_cJSON_ArrayForEach(a_element, a)
            {
                /* TODO This has O(n^2) runtime, which is horrible! */
                b_element = get_object_item(b, a_element->string, case_sensitive);
                if (b_element == NULL)
                {
                    return FALSE;
                }

                if (!cJSON_Compare(a_element, b_element, case_sensitive))
                {
                    return FALSE;
                }
            }

            /* doing this twice, once on a and b to prevent TRUE comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
            QUOS_cJSON_ArrayForEach(b_element, b)
            {
                a_element = get_object_item(a, b_element->string, case_sensitive);
                if (a_element == NULL)
                {
                    return FALSE;
                }

                if (!cJSON_Compare(b_element, a_element, case_sensitive))
                {
                    return FALSE;
                }
            }

            return TRUE;
        }

        default:
            return FALSE;
    }
}
