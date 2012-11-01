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
 * File name : dahdi.h
 * Created on: Aug 27, 2012
 * Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *	       Based on dahdi.h by Wagner Gegler
 *
 * Definitions for dahdi.h
 */

#ifndef DAHDI_H_
#define DAHDI_H_

/*
 * General Includes
 */
#include <dahdi/ctd_user.h>


/*
 * General Defines
 */

#define FILE_DAHDI_SYSTEM_CONF	"/etc/dahdi/system.conf"
#define FILE_CHAN_DAHDI_CONF	"/etc/asterisk/chan_dahdi.conf"
#define FILE_DAHDI_IOCTL	"/dev/dahdi/ctl"
#define FILE_AISDAEMON_CONF	"/etc/dahdi/aisdaemon.conf"

#define DAHDI_SYSTEM_CONTENT	"loadzone=br\n" \
				"defaultzone=br\n"

#define CHAN_DAHDI_CONTENT	"[channels]\n" \
				"language=br\n" \
				"usecallerid=yes\n" \
				"echocancelwhenbridged=no\n" \
				"buffers=8,half\n" \
				"context=from-e1\n" \
				"mfcr2_variant=br\n" \
				"mfcr2_category=national_subscriber\n" \
				"mfcr2_mfback_timeout=5000\n" \
				"mfcr2_metering_pulse_timeout=1000\n" \
				"#include chan_dahdi_custom.conf\n"

#define MAX_SPANS	1
#define NUMSPANS	1
#define BUF_SIZE	256

/* Dahdi Switchtype References - related to spec -- parks_apvs_desc_v03.docx */
#define Q_SIG_NAME				"qsig"		//Q.SIG
#define Q_SIG_COD				40
#define NATIONAL_ISDN_1_NAME	"ni1"		//Old National ISDN 1
#define NATIONAL_ISDN_1_COD		20
#define LUCENT_5ESS_NAME		"5ess"		//Lucent 5ESS
#define LUCENT_5ESS_COD			10
#define ATeT_4ESS_NAME			"4ess"		//AT&T 4ESS
#define ATeT_4ESS_COD			8
#define NORTEL_DMS100_NAME		"dms100"	//Nortel DMS100
#define NORTEL_DMS100_COD		4
#define NATIONAL_ISDN2_NAME		"national"	//National ISDN 2
#define NATIONAL_ISDN2_COD		2
#define EURO_ISDN_NAME			"euroisdn"	//EuroISDN (common in Europe)
#define EURO_ISDN_COD			1




/*
 * General Structures
 */
typedef enum {
	DAHDI_SIG_MFCR2 = 0,
	DAHDI_SIG_ISDN_NET,
	DAHDI_SIG_ISDN_CPE,
} dahdi_sig;

struct libamg_dahdi_isdn {
	int overlapdial;
	char switchtype[16];
};

struct libamg_dahdi_mfcr2 {
	int get_ani_first;
	int max_ani;
	int max_dnis;
	int allow_collect_calls;
	int double_answer;
};

struct libamg_dahdi_span {
	int enable;
	int channels;	/* Number of channels */
	int clock;	/* Clock source (0, 1, etc.) */
	int crc;
	int signalling;
	int echocancel;
	struct libamg_dahdi_isdn isdn;
	struct libamg_dahdi_mfcr2 mfcr2;
};

struct libamg_dahdi_config {
	struct libamg_dahdi_span spans[MAX_SPANS];
};

struct libamg_dahdi_span_status {
	char *alarms_str;
	struct ctd_span_stats stats;
};

struct libamg_dahdi_status {
	struct libamg_dahdi_span_status spans[MAX_SPANS];
};


/*
 * Functions Declaration
 */

/**
 * libamg_dahdi_get_switchtype_name
 *
 * Get Dahdi switch type name by addressed code in spec.
 *
 * @param switchtype_code
 * @return string
 */
char * libamg_dahdi_get_switchtype_name (int switchtype_code);

/**
 * libamg_dahdi_get_status
 *
 * Get Dahdi system status.
 *
 * @return struct libamg_dahdi_status *
 */
struct libamg_dahdi_status *libamg_dahdi_get_status(void);

/**
 * libamg_dahdi_reset_status
 *
 * Reset Dahdi system status.
 *
 */
void libamg_dahdi_reset_status(void);

/**
 * libamg_dahdi_parse_config
 *
 * Get Dahdi configs through  libamg_dahdi_config structure.
 *
 * @return struct libamg_dahdi_config *
 */
struct libamg_dahdi_config *libamg_dahdi_parse_config(void);

/**
 * libamg_dahdi_save_config
 *
 * Saves Dahdi configs at libamg_dahdi_config structure
 * inside dahdi configuration file.
 *
 * @param conf
 * @return -1 if error, 0 if OK
 */
int libamg_dahdi_save_config(struct libamg_dahdi_config *conf);

/**
 * libamg_ais_daemon_check
 *
 * Verify if the AIS daemon alarm is enabled inside the config file.
 *
 * @return -1 if error, 0 if AIS is not activated, 1 if AIS is activated
 */
int libamg_ais_daemon_check(void);

/**
 * libamg_ais_daemon_set
 *
 * Enable/Disable AIS daemon alarm.
 *
 * @param enable
 * @return -1 if error, 0 if OK.
 */
int libamg_ais_daemon_set(int enable);

/**
 * libamg_ais_daemon_apply_config
 *
 * Apply AIS daemon configs.
 *
 * @return
 */
int libamg_ais_daemon_apply_config(void);

/**
 * libamg_dahdi_apply_config
 *
 * Apply Dahdi configs.
 *
 */
void libamg_dahdi_apply_config(void);

/**
 * libamg_dahdi_flash_save_config
 *
 * Saves Dahdi configuration file to flash.
 *
 */
void libamg_dahdi_flash_save_config(void);

/**
 * libamg_dahdi_ais_get
 *
 * Get Dahdis AIS status.
 *
 * @param span
 * @return
 */
int libamg_dahdi_ais_get(int span);

/**
 * libamg_dahdi_ais_set
 *
 * Set Dahdis AIS.
 *
 * @param span
 * @param enable
 * @return -1 if error, 0 if OK.
 */
int libamg_dahdi_ais_set(int span, int enable);
#endif /* DAHDI_H_ */
