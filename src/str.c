/*
 * Copyright (c) 2012 PD3 Tecnologia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  File name: str.c
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

void libamg_str_striplf(char *str)
{
	int ln = strlen(str);; /* string length */

	/* Removes anything below 32 (ascii) */
	while ((ln > 0) && (str[ln - 1] < 32))
		str[--ln] = 0;
}

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
