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

/* Misc */
#define FILE_SIP_CONF_NAME			"sip.conf"
#define FILE_SIP_CONF_PATH			"/etc/asterisk/"
#define FILE_SIP_CONF				FILE_SIP_CONF_PATH FILE_SIP_CONF_NAME
#define FILE_SIP_CONF_DEFAULT_PATH	"/etc.ro/asterisk/"

#define PRODUCT_NAME	"Parks-AMG"

#define NUM_SIP_ACCOUNTS	1

#define SIP_GENERAL_CONTENT	"[general]\n" \
							"context=from-sip\n" \
							"sdpsession=SIP Call\n" \
							"sdpowner=ParksSIP-MG\n" \
							"disallow=all\n" \
							"srvlookup=yes\n" \
							"bindaddr=0.0.0.0\n" \
							"engine=comcerto\n"

#define SIP_ACCOUNT_CONTENT	"#include sip_account_custom.conf\n" \
							"type=friend\n"

#define SIP_ACCOUNT_NAME "User1"

#define SIP_TCP_BINDADDR "tcpbindaddr=0.0.0.0\n"

#define NUM_AVAILABLE_CODECS 9
#define ON	1
#define OFF	0
#define DISABLE -1
#define TCP_UDP_ADDR_DEFAULT "0.0.0.0"

/* Codecs */
#define CODEC_G711_A_NAME			"alaw"
#define CODEC_G711_A_COD			0x100
#define CODEC_G711_U_NAME			"ulaw"
#define CODEC_G711_U_COD			0x80
#define CODEC_G723_1_NAME			"g723"
#define CODEC_G723_1_COD			0x40
#define CODEC_G726_16Kbps_NAME 		"g726-16"
#define CODEC_G726_16Kbps_COD		0x20
#define CODEC_G726_24Kbps_NAME 		"g726-24"
#define CODEC_G726_24Kbps_COD		0x10
#define CODEC_G726_32Kbps_NAME 		"g726"
#define CODEC_G726_32Kbps_COD		0x8
#define CODEC_G726_40Kbps_NAME 		"g726-40"
#define CODEC_G726_40Kbps_COD		0x4
#define CODEC_G729_NAME				"g729"
#define CODEC_G729_COD				0x2
#define CODEC_GSM_NAME				"gsm"
#define CODEC_GSM_COD				0x1

/* DTMF Modes*/
#define DTMF_MODE_IN_BAND_NAME		"inband"
#define DTMF_MODE_IN_BAND_COD		0x4
#define DTMF_MODE_RFC2833_NAME		"rfc2833"
#define DTMF_MODE_RFC2833_COD		0x2
#define DTMF_MODE_SIP_INFO_NAME		"info"
#define DTMF_MODE_SIP_INFO_COD		0x1

/* Insecure Modes*/
#define INSECURE_MODE_VERY_NAME			"very"
#define INSECURE_MODE_VERY_COD			0x8
#define INSECURE_MODE_PORT_NAME			"port"
#define INSECURE_MODE_PORT_COD			0x4
#define INSECURE_MODE_INVITE_NAME		"invite"
#define INSECURE_MODE_INVITE_COD		0x2
#define INSECURE_MODE_PORT_INVITE_NAME	"port,invite"
#define INSECURE_MODE_PORT_INVITE_COD	0x1
#define INSECURE_MODE_NO_NAME			"no"
#define INSECURE_MODE_NO_COD			0x0

/* Transport Modes*/
#define TRANSPORT_MODE_UDP_NAME	"udp"
#define TRANSPORT_MODE_UDP_COD	0
#define TRANSPORT_MODE_TCP_NAME	"tcp"
#define TRANSPORT_MODE_TCP_COD	1

/* Session Timers Modes */
#define SESSION_TIMERS_MODE_ORIG_NAME	"originate"
#define SESSION_TIMERS_MODE_ORIG_COD	0x4
#define SESSION_TIMERS_MODE_REF_NAME	"refuse"
#define SESSION_TIMERS_MODE_REF_COD		0x2
#define SESSION_TIMERS_MODE_ACC_NAME	"accept"
#define SESSION_TIMERS_MODE_ACC_COD		0x1
#define SESSION_TIMERS_MODE_OFF_COD		0x0

/* Session Timers Refresher Modes */
#define SESSION_TIMERS_REFRESHER_MODE_UAC_NAME		"uac"
#define SESSION_TIMERS_REFRESHER_MODE_UAC_COD		0x2
#define SESSION_TIMERS_REFRESHER_MODE_UAS_NAME		"uas"
#define SESSION_TIMERS_REFRESHER_MODE_UAS_COD		0x1
#define SESSION_TIMERS_REFRESHER_MODE_OFF_COD		0x0

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
	char transport[8];
	unsigned short port;
	char dtmfmode[32];
	int allow[NUM_AVAILABLE_CODECS];
};

struct libamg_sip_config {
	unsigned short bindport;
	int tcpenable;
	int t1min;
	int timert1;
	int timerb;
	int rtptimeout;
	int rtpholdtimeout;
	int rtpkeepalive;
	char session_timers[16];
	int session_expires;
	int session_minse;
	char session_refresher[8];
	int defaultexpiry;
	int register_enable;
	int autoframing;
	libamg_jb_config jb_conf;
	char register_username[64];
	char register_secret[64];
	char register_host[64];
	struct libamg_sip_account accounts[NUM_SIP_ACCOUNTS];
};


/*
 * Functions Declaration
 */

/**
 * libamg_sip_get_min_codec_intvl
 *
 * Get Minimum Codec RTP Interval
 *
 * @return int value of min codec intvl, negative if error
 */
int libamg_sip_get_min_codec_intvl(void);

/**
 * libamg_sip_reset_config
 *
 * Restore the SIP configures from default sip configuration file
 *
 * @return 0 if success, negative if error
 */
int libamg_sip_reset_config(void);

/**
 * libamg_sip_get_session_timers_refresher_code
 *
 * Get SIP session_timers_refresher code by addressed name in spec.
 *
 * @param session_timers_refresher_name
 * @return int
 */
int libamg_sip_get_session_timers_refresher_code(const char *session_timers_refresher_name);

/**
 * libamg_sip_get_session_timers_refresher_name
 *
 * Get SIP session_timers_refresher name by addressed code in spec.
 *
 * @param session_timers_refresher_code
 * @return string
 */
char * libamg_sip_get_session_timers_refresher_name(int session_timers_refresher_code);

/**
 * libamg_sip_get_session_timers_code
 *
 * Get SIP session_timers code by addressed name in spec.
 *
 * @param session_timers_name
 * @return int
 */
int libamg_sip_get_session_timers_code(const char *session_timers_name);

/**
 * libamg_sip_get_session_timers_name
 *
 * Get SIP session_timers name by addressed code in spec.
 *
 * @param session_timers_code
 * @return string
 */
char * libamg_sip_get_session_timers_name(int session_timers_code);

/**
 * libamg_sip_get_transport_code
 *
 * Get SIP transport code by addressed name in spec.
 *
 * @param transport_name
 * @return int
 */
int libamg_sip_get_transport_code(const char *transport_name);

/**
 * libamg_sip_get_transport_name
 *
 * Get SIP transport name by addressed code in spec.
 *
 * @param transport_code
 * @return string
 */
char * libamg_sip_get_transport_name(int transport_code);

/**
 * libamg_sip_get_insecure_code
 *
 * Get SIP insecure code by addressed name in spec.
 *
 * @param insecure_name
 * @return int
 */
int libamg_sip_get_insecure_code(const char *insecure_name);

/**
 * libamg_sip_get_insecure_name
 *
 * Get SIP insecure name by addressed code in spec.
 *
 * @param insecure_code
 * @return string
 */
char * libamg_sip_get_insecure_name(int insecure_code);

/**
 * libamg_sip_get_dtmfmode_code
 *
 * Get SIP dtmf_mode code by addressed name in spec.
 *
 * @param dtmfmode_name
 * @return int
 */
int libamg_sip_get_dtmfmode_code(const char *dtmfmode_name);

/**
 * libamg_sip_get_dtmfmode_name
 *
 * Get SIP dtmf_mode name by addressed code in spec.
 *
 * @param dtmfmode_code
 * @return string
 */
char * libamg_sip_get_dtmfmode_name(int dtmfmode_code);


/**
 * libamg_sip_get_codec_code
 *
 * Get SIP codec code by addressed name in spec.
 *
 * @param codec_name
 * @return int
 */
int libamg_sip_get_codec_code(const char *codec_name);

/**
 * libamg_sip_get_codec_name
 *
 * Get SIP codec name by addressed code in spec.
 *
 * @param codec_code
 * @return string
 */
char * libamg_sip_get_codec_name(int codec_code);

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
 * libamg_sip_get_register
 *
 * Gets the 'register' status from Asterisk.
 *
 * @param response
 * @param maxlen
 * @return -1 for an error, 0 if register not OK, 1 of register OK.
 */
int libamg_sip_get_register(char *response, int maxlen);

#endif /* SIP_H_ */
