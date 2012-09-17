/*
 * sip.h
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


/*
 * General Structures
 */
struct libamg_sip_account {
#ifdef NOT_SUPPORTED_AMG
	int register_enable;
#endif
	char username[32];
	char secret[32];
	char host[32];
	unsigned short port;
	char callerid[32];
	char fromuser[32];
	char insecure[32];
	char dtmfmode[32];
	char allow[32];
	int nat;
	int qualify;
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
