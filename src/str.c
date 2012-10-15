/*
 * str.c
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *      Based on str.c by Wagner Gegler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "str.h"

char *libamg_str_skip_spaces(const char *str)
{
	while (isspace(*str))
		str++;
		
	return (char*) str;
}

char *libamg_str_strim(char *s)
{
	size_t size;
	char *end;

	s = libamg_str_skip_spaces(s);
	size = strlen(s);
	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return s;
}

char *libamg_str_next_token(char *str, char sep)
{
	char *token = strchr(str, sep);
	if (!token)
		return token;
	token[0] = '\0';
	return ++token;
}
