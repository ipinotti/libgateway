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
 *  File name: str.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *      Based on str.h by Wagner Gegler
 */

#ifndef STR_H_
#define STR_H_

/*
 * Functions Declaration
 */

/**
 * libamg_str_striplf
 *
 * Removes line feed from string
 *
 * @param str
 */
void libamg_str_striplf(char *str);

/**
 * libamg_str_skip_spaces
 *
 * Skip spaces from a given string.
 *
 * @param str
 * @return char * result
 */
char *libamg_str_skip_spaces(const char *str);

/**
 * libamg_str_strim
 *
 * Trim desired part of a given string.
 *
 * @param s
 * @return char * result
 */
char *libamg_str_strim(char *s);

/**
 * libamg_str_next_token
 *
 * Returns next token from a string
 * Replace separator to a \0
 * Returns pointer after separator
 *
 * @param str
 * @param sep
 * @return char * result
 */
char *libamg_str_next_token(char *str, char sep);


#endif /* STR_H_ */

