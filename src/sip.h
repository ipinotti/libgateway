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
 *  File name: sip.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *      Based on sip.h by Wagner Gegler
 */

#ifndef SIP_H_
#define SIP_H_


/*
 * General Includes
 */
#include "comcerto.h"


/*
 * General Defines
 */
#define FILE_SIP_CONF		"/etc/asterisk/sip.conf"

#define PRODUCT_NAME	"Parks-AMG"

#define NUM_SIP_ACCOUNTS	1


#define SIP_GENERAL_CONTENT	"[general]\n" \
				"context=from-sip\n" \
				"disallow=all\n" \
				"bindaddr=0.0.0.0\n"

#define SIP_ACCOUNT_CONTENT	"\n[User1]\n" \
				"#include sip_account_custom.conf\n" \
				"type=friend\n"

/* Codecs */
#define G711_A "alaw"
#define G711_U "ulaw"
#define G723_1 "g723"
#define G726_16Kbps "g726-16"
#define G726_24Kbps "g726-24"
#define G726_32Kbps "g726-32"
#define G726_40Kbps "g726-40"
#define G729 "g729"
#define GSM "gsm"


/*
 * General Structures
 */

typedef struct libamg_sip_codecs {
	unsigned int gsm:1;
	unsigned int g729:1;
	unsigned int g726_40kbps:1;
	unsigned int g726_32kbps:1;
	unsigned int g726_24kbps:1;
	unsigned int g726_16kbps:1;
	unsigned int g723_1:1;
	unsigned int g711_U:1;
	unsigned int g711_A:1;
}libamg_sip_codecs;

struct libamg_sip_account {
#ifdef NOT_SUPPORTED_AMG
	int register_enable;
	int nat;
	int qualify;
	char callerid[32];
	char fromuser[32];
	char insecure[32];
#endif
	char username[32];
	char secret[32];
	char host[32];
	unsigned short port;
	char dtmfmode[32];
	libamg_sip_codecs allow;
};

struct libamg_sip_config {
	unsigned short bindport;
	libamg_jb_config jb_conf;
	struct libamg_sip_account accounts[NUM_SIP_ACCOUNTS];
};


/*
 * Functions Declaration
 */

/**
 * libamg_sip_parse_config
 *
 * Get SIP configs through libamg_sip_config structure.
 *
 * @return struct libamg_sip_config *
 */
struct libamg_sip_config *libamg_sip_parse_config(void);

/**
 * libamg_sip_save_config
 *
 * Saves SIP configs at libamg_sip_config structure
 * inside sip configuration file.
 *
 * @param conf
 * @return -1 if error, 0 if OK
 */
int libamg_sip_save_config(struct libamg_sip_config *conf);

/**
 * libamg_sip_flash_save_config
 *
 * Saves SIP configuration file to flash.
 *
 */
void libamg_sip_flash_save_config(void);

/**
 * libamg_asterisk_get_register
 *
 * Gets the 'register' status from Asterisk.
 *
 * @param response
 * @param maxlen
 * @return -1 for an error, 0 if register not OK, 1 of register OK.
 */
int libamg_asterisk_get_register(char *response, int maxlen);

#endif /* SIP_H_ */
