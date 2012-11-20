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
							"srvlookup=yes\n" \
							"bindaddr=0.0.0.0\n" \
							"engine=comcerto\n"

#define SIP_ACCOUNT_CONTENT	"#include sip_account_custom.conf\n" \
							"type=friend\n"

#define SIP_ACCOUNT_NAME "User1"

#define SIP_TCP_BINDADDR "tcpbindaddr=0.0.0.0\n"

/* Codecs */
#define CODEC_G711_A 			"alaw"
#define CODEC_G711_A_COD		0x100
#define CODEC_G711_U 			"ulaw"
#define CODEC_G711_U_COD		0x80
#define CODEC_G723_1 			"g723"
#define CODEC_G723_1_COD		0x40
#define CODEC_G726_16Kbps 		"g726-16"
#define CODEC_G726_16Kbps_COD	0x20
#define CODEC_G726_24Kbps 		"g726-24"
#define CODEC_G726_24Kbps_COD	0x10
#define CODEC_G726_32Kbps 		"g726-32"
#define CODEC_G726_32Kbps_COD	0x8
#define CODEC_G726_40Kbps 		"g726-40"
#define CODEC_G726_40Kbps_COD	0x4
#define CODEC_G729 				"g729"
#define CODEC_G729_COD			0x2
#define CODEC_GSM 				"gsm"
#define CODEC_GSM_COD			0x1

/* DTMF Modes*/
#define DTMF_MODE_IN_BAND			"inband"
#define DTMF_MODE_IN_BAND_COD		0x4
#define DTMF_MODE_RFC2833			"rfc2833"
#define DTMF_MODE_RFC2833_COD		0x2
#define DTMF_MODE_SIP_INFO			"info"
#define DTMF_MODE_SIP_INFO_COD		0x1

#define NUM_AVAILABLE_CODECS 9
#define ON	1
#define OFF	0
#define TCP_UDP_ADDR_DEFAULT "0.0.0.0"
#define TRANSPORT_TCP	"tcp"
#define TRANSPORT_UDP	"udp"


/*
 * General Structures
 */

struct libamg_sip_account {
	int nat;
	int qualify;
	char callerid[64];
	char fromuser[64];
	char insecure[32];
	char username[64];
	char secret[64];
	char host[64];
	char transport[6];
	unsigned short port;
	char dtmfmode[32];
	int allow[NUM_AVAILABLE_CODECS];
};

struct libamg_sip_config {
	unsigned short bindport;
	int tcpenable;
	int defaultexpiry;
	int register_enable;
	char register_username[64];
	char register_secret[64];
	libamg_jb_config jb_conf;
	struct libamg_sip_account accounts[NUM_SIP_ACCOUNTS];
};


/*
 * Functions Declaration
 */

/**
 * libamg_sip_get_codec_name
 *
 * Get SIP codec name by addressed code in spec.
 *
 * @param codec_code
 * @return string
 */
char * libamg_sip_get_codec_name (int codec_code);

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
