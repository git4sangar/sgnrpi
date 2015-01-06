/*
 * sgn_string.c
 *
 *  SGN
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "sgn_string.h"
#include "sgn_types.h"

#define DEFAULT_SIZE  128

/**
 * These functions are similar to the glib equivalents but do not handle embedded null characters.
 */

sgn_string_t *sgn_string_new(const char *init) {

	/* allocate the structure */
	sgn_string_t *p_string = calloc(sizeof(sgn_string_t), 1);
	if (p_string == NULL) {
		return NULL;
	}
	p_string->len = 0;
	p_string->allocated_len = 0;
	p_string->str = NULL;

	/* how big is the init string */
	int init_len = 0;
	int full_len = 0;
	if (init != NULL) {
		init_len = strlen(init);
		full_len = init_len + 1;
	}

	/* double buffer size till we are above init size */
	int alloc_len = DEFAULT_SIZE;
	while (alloc_len < full_len) {
		alloc_len *= 2;
	}

	/* allocate the buffer */
	p_string->str = calloc(alloc_len, 1);
	if (p_string->str == NULL) {
		/* could not get memory, just return the struct */
		return p_string;
	}
	p_string->allocated_len = alloc_len;

	/* null terminate the buffer */
	*p_string->str = '\0';

	/* copy init string if any */
	if (init != NULL) {
		strcpy(p_string->str, init);
		p_string->len = init_len;
	}

	return p_string;
}

sgn_string_t *sgn_string_sized_new(int initial_size) {

	/* allocate the structure */
	sgn_string_t *p_string = calloc(sizeof(sgn_string_t), 1);
	if (p_string == NULL) {
		return NULL;
	}
	p_string->len = 0;
	p_string->allocated_len = 0;
	p_string->str = NULL;

	int len = 0;
	if (initial_size > 0) {
		len = initial_size;
	} else {
		len = DEFAULT_SIZE;
	}

	/* allocate the buffer */
	p_string->str = calloc(len, 1);
	if (p_string->str == NULL) {
		/* could not get memory, just return the struct */
		return p_string;
	}
	p_string->allocated_len = len;

	/* null terminate the buffer */
	*p_string->str = '\0';

	return p_string;
}

char *sgn_string_free(sgn_string_t *p_string, int free_segment) {

	char *ret = NULL;

	/* if struct is null return null */
	if (p_string == NULL) {
		return ret;
	}

	/* if there is a buffer ... */
	if (p_string->str != NULL) {

		/* if we are not freeing the segment save a pointer to the buffer
		 * otherwise free the buffer. */
		if (free_segment == sgn_false) {
			ret = p_string->str;
		} else {
			free(p_string->str); p_string->str = NULL;
		}
	}

	/* free the structure */
	free(p_string); p_string = NULL;

	return ret;
}

sgn_string_t *sgn_string_append(sgn_string_t *p_string, const char *val) {
	sgn_string_t *ret = NULL;

	/* if struct is null */
	if (p_string == NULL || val == NULL) {
		return ret;
	}

	/* setup to return the struct */
	ret = p_string;

	/* see if we have anything to append */
	if (val == NULL) {
		return ret;
	}

	/* figure out if the buffer size needs to be bigger and realloc if needed */
	int new_len = p_string->len + strlen(val);
	int needed_len = new_len + 1;
	if (needed_len > p_string->allocated_len) {
		int new_allocated_len = p_string->allocated_len;
		do {
			new_allocated_len *= 2;
		} while (needed_len > new_allocated_len);

		p_string->str = realloc(p_string->str, new_allocated_len);
		if (p_string->str != NULL) {
			p_string->allocated_len = new_allocated_len;
		} else {
			/* realloc failed so our allocated length is zero */
			p_string->allocated_len = 0;
		}
	}

	/* if we still have a buffer append the string */
	if (p_string->str != NULL) {
		strcpy(p_string->str + p_string->len, val);
		p_string->len = new_len;
	}

	return ret;
}

sgn_string_t *sgn_string_append_printf(sgn_string_t *p_string, const char *format, ...) {
	sgn_string_t *ret = NULL;

	/* if struct is null return null */
	if (p_string == NULL) {
		return ret;
	}

	/* setup to return the struct */
	ret = p_string;

	/* if the format is null just return what we got */
	if (format == NULL) {
		return ret;
	}

	/* format the string to append */
    va_list ap;
    va_start(ap, format);
    char *val;
    int len = vasprintf(&val, format, ap);
    va_end(ap);

    /* if we did not get an error use sgn_string_append */
    if (len != -1) {
    	ret = sgn_string_append(ret, val);

    	/* free the returned string */
    	free(val); val = NULL;
    }

	return ret;
}

sgn_byte_array_t *sgn_byte_array_sized_new( int i32Size )
{
	int len;

	sgn_byte_array_t	*pByteArray	= calloc(sizeof(sgn_byte_array_t), 1);
	if( NULL == pByteArray )
		return NULL;

	pByteArray->allocated_len	= 0;
	pByteArray->len				= 0;
	pByteArray->str				= NULL;

	if ( 0 < i32Size )
		len = i32Size;
	else
		len = DEFAULT_SIZE;

	pByteArray->str		= calloc(len, 1);
	if( NULL == pByteArray->str )
		return pByteArray;

	pByteArray->allocated_len	= len;
	return pByteArray;
}

sgn_byte_array_t	*sgn_byte_array_append( sgn_byte_array_t *pByteArray, char *pData, int i32Size )
{
	int		i32ReqdLen, i32NewLen = 0;

	if( NULL == pByteArray || NULL == pData || 0 >= i32Size )
		return pByteArray;

	i32ReqdLen	= pByteArray->len + i32Size;
	if( i32ReqdLen > pByteArray->allocated_len )
	{
		i32NewLen	=	pByteArray->allocated_len;
		do{
			i32NewLen	= i32NewLen * 2;
		}while( i32ReqdLen > i32NewLen  );

		pByteArray->str		= realloc( pByteArray->str, i32NewLen );
		if( pByteArray->str )
			pByteArray->allocated_len	= i32NewLen;
		else
			pByteArray->allocated_len	= 0;
	}

	if( pByteArray->str )
	{
		memcpy( pByteArray->str + pByteArray->len, pData, i32Size );
		pByteArray->len		= i32ReqdLen;
	}
	return pByteArray;
}

unsigned char *sgn_byte_array_free( sgn_byte_array_t *pByteArray, sgn_bool isSegmentToBeFreed )
{
	unsigned char	*pSegment = NULL;

	if( NULL == pByteArray )
		return NULL;

	if( pByteArray->str )
	{
		if( isSegmentToBeFreed )
			free( pByteArray->str );
		else
			pSegment = pByteArray->str;
	}

	free( pByteArray ); pByteArray = NULL;
	return pSegment;
}

char *sgn_trim_string(char *pStr) {

	char *pLTrim = NULL, *pRTrim = NULL;
	if( NULL == pStr )
		return NULL;

	pLTrim = pStr; /* Trim left hand side */
	while(*pLTrim && (' ' == *pLTrim || '\r' == *pLTrim || '\n' == *pLTrim || '\t' == *pLTrim || '\v' == *pLTrim) )
		pLTrim++;

	if(*pLTrim) {
		pRTrim = pLTrim + strlen(pLTrim) - 1;	/* Trim right hand side */
		while(*pRTrim && (' ' == *pRTrim || '\r' == *pRTrim || '\n' == *pRTrim || '\t' == *pRTrim || '\v' == *pRTrim) )
			pRTrim--;

		/* If, either Left trim or Right tirm has happened, then move the string */
		if(pStr != pLTrim || '\0' != *(pRTrim+1)) {
			*(pRTrim+1) = '\0';
			memmove(pStr, pLTrim, strlen(pLTrim)+1);
			pLTrim = pStr;
		}
	}
	/* return NULL if pointing to null-string. */
	return ('\0' == *pLTrim) ? NULL: pLTrim;
}

char *sgn_strdup(const char *pStr) {
	char *pSzTemp = NULL;

	if (pStr) {
		pSzTemp = calloc(strlen(pStr) + 1, 1);
		if (pSzTemp) {
			strcpy( pSzTemp, pStr );
		}
	}
	return pSzTemp;
}

int sgn_strlen(const char *pStr) {
	if(NULL == pStr) {
		return 0;
	}
	return strlen(pStr);
}

int sgn_strcmp( const char *pA, const char *pB ) {
	if(!pA || !pB)
		return 1;
	return strcmp(pA, pB);
}

int sgn_strtol(const char *pStr) {
	return (pStr)? strtol(pStr, NULL, 10) : 0;
}

char *sgn_strstr(const char *pStrMain, const char *pStrSub) {
	if(NULL == pStrMain || NULL == pStrSub) {
		return NULL;
	}
	return strstr(pStrMain, pStrSub);
}

char *sgn_replace_string(char *pString, char *pattern, char *replace){

	char *ch, *buffer;

	if(!pString || !pattern || !replace)
		return pString;

	if(!(ch = strstr(pString,pattern))){
		return pString;
	}

	buffer = calloc((strlen(pString) - strlen(pattern) + strlen(replace) + 1), 1);

	strncpy( buffer, pString, ch-pString );
	buffer[ ch-pString ]=0;
	sprintf( buffer+(ch-pString), "%s%s", replace, ch+strlen(pattern));

	free(pString);

	return buffer;
}

char *sgn_get_key_value(char *pContent, char *pKey) {
	char *strtokptr = NULL;
	char *delim		= "&=";
	if(!pContent || !pKey) {
		return NULL;
	}
	while(1) {
		char *pTok1 = strtok_r(pContent, delim, &strtokptr);
		pContent	= NULL;
		if(NULL == pTok1) {
			return NULL;
		}
		if(0 == strcmp(pKey, pTok1)) {
			char *pVal	= strtok_r(pContent, delim, &strtokptr);
			return pVal;
		}
	}
	return NULL;
}

char *sgn_pop_key_value(char *pContent, char *pKey) {
	char *strtokptr = NULL;
	char *delim		= "&";
	sgn_string_t *pStr= sgn_string_new(NULL);

	if(!pContent || !pKey) {
		return NULL;
	}
	while(1) {
		char *pTok1 = strtok_r(pContent, delim, &strtokptr);
		pContent	= NULL;
		if(NULL == pTok1) {
			break;
		}
		if(NULL == strstr(pTok1, pKey)) {
			sgn_string_append(pStr, "&");
			sgn_string_append(pStr, pTok1);
		}
	}
	char *pRet	= pStr->str;
	sgn_string_free(pStr, 0);
	return pRet;
}
