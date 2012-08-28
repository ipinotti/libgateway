/*
 * str.h
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

