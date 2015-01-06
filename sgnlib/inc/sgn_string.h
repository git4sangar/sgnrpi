/*
 * sgn_string.h
 * SGN
 */

#ifndef YSTRING_H_
#define YSTRING_H_

#include "sgn_types.h"

typedef struct sgn_byte_array
{
	unsigned char	*str;
	int		len;
	int		allocated_len;
} sgn_byte_array_t;

typedef struct sgn_string {
	char *str;
	int len;
	int allocated_len;
} sgn_string_t;

sgn_string_t *sgn_string_new(const char *init);
sgn_string_t *sgn_string_sized_new(int initial_size);
char *sgn_string_free(sgn_string_t *p_string, int free_segment);
sgn_string_t *sgn_string_append(sgn_string_t *p_string, const char *val);
sgn_string_t *sgn_string_append_printf(sgn_string_t *p_string, const char *format, ...);
char *sgn_strdup(const char *pStr);
int sgn_strcmp( const char *pStrA, const char *pStrB );
int sgn_strtol(const char *pStr);
char *sgn_strstr(const char *pStrMain, const char *pStrSub);
char *sgn_trim_string(char *pStr);
char *sgn_replace_string(char *pString, char *pattern, char *replace);
sgn_byte_array_t *sgn_byte_array_sized_new( int i32Size );
unsigned char *sgn_byte_array_free( sgn_byte_array_t *pByteArray, sgn_bool isSegmentToBeFreed );
sgn_byte_array_t	*sgn_byte_array_append( sgn_byte_array_t *pByteArray, char *pData, int i32Size );
char *sgn_get_key_value(char *pContent, char *pKey);
char *sgn_pop_key_value(char *pContent, char *pKey);
int sgn_strlen(const char *pStr);

#endif /* YSTRING_H_ */
