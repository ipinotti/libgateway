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
 *  File name: extensions.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *
 */

#ifndef EXTENSIONS_H_
#define EXTENSIONS_H_


/*
 * General Includes
 */
#include "comcerto.h"


/*
 * General Defines
 */

#define FILE_EXTENSIONS_CONF_NAME			"extensions.conf"
#define FILE_EXTENSIONS_CONF_PATH			"/etc/asterisk/"
#define FILE_EXTENSIONS_CONF				FILE_EXTENSIONS_CONF_PATH FILE_EXTENSIONS_CONF_NAME
#define FILE_EXTENSIONS_CONF_DEFAULT_PATH	"/etc.ro/asterisk/"

#define PRODUCT_NAME	"Parks-AMG"

#define EXTENSIONS_GENERAL_CONTENT	"[general]\n" \
									"static=yes\n" \
									"writeprotect=no\n" \
									"autofallthrough=yes\n" \
									"clearglobalvars=no\n" \
									"priorityjumping=no\n"

#define EXTENSIONS_GLOBALS_CONTENT	"\n[globals]" \

#define EXTENSIONS_GLOBALS_PREFIX	"PREFIX="

#define EXTENSIONS_ACCOUNT_FROM_SPAN1_CONTENT	"\n[from-span1]\n" \
												"exten => _X.,1,Noop()\n" \
												"exten => _X.,n,Answer()\n" \
												"exten => _X.,n,Ringing()\n"

#define EXTENSIONS_ACCOUNT_FROM_SPAN1_DIAL_STAND	"exten => _X.,n,Dial(SIP/${EXTEN}@User1&Local/s@nop)\n"
#define EXTENSIONS_ACCOUNT_FROM_SPAN1_DIAL_PREFIX	"exten => _X.,n,Dial(SIP/${PREFIX}${EXTEN}@User1&Local/s@nop)\n"

#define EXTENSIONS_ACCOUNT_NOP_CONTENT	"\n[nop]\n" \
										"exten => s,1,Hangup()\n"

#define EXTENSIONS_ACCOUNT_FROM_SIP_CONTENT	"\n[from-sip]\n" \
											"exten => _X.,1,Noop()\n" \
											"exten => _X.,n,Dial(DAHDI/g1/${EXTEN})\n"

/*
 * General Structures
 */

struct libamg_extensions_config {
	int prefix_enable;
	int prefix;
};


/*
 * Functions Declaration
 */

/**
 * libamg_extensions_reset_config
 *
 * Restore the extensions configures from default extensions configuration file
 *
 * @return 0 if success, negative if error
 */
int libamg_extensions_reset_config(void);

/**
 * libamg_extensions_parse_config
 *
 * Get extensions configs through libamg_extensions_config structure.
 *
 * @return struct libamg_extensions_config *
 */
struct libamg_extensions_config *libamg_extensions_parse_config(void);

/**
 * libamg_extensions_save_config
 *
 * Saves extensions configs at libamg_extensions_config structure
 * inside extensions configuration file.
 *
 * @param conf
 * @return -1 if error, 0 if OK
 */
int libamg_extensions_save_config(struct libamg_extensions_config *conf);

/**
 * libamg_extensions_flash_save_config
 *
 * Saves extensions configuration file to flash.
 *
 */
void libamg_extensions_flash_save_config(void);

#endif /* EXTENSIONS_H_ */
