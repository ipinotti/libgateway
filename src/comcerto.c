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
 * File name: protocol.c
 * Created on: Sep 5, 2012
 * Author: Igor Kramer Pinotti
 *
 * Configuration functions for comcerto.conf parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <regex.h>

#include "log.h"
#include "str.h"
#include "comcerto.h"
#include "sip.h"

#define BUF_SIZE 128

int libamg_comcerto_get_cng_enable(void)
{
	int cng_enable = 0;
	struct libamg_comcerto_config *config = libamg_comcerto_parse_config();

	cng_enable = config->cng_enable;
	free(config);

	return cng_enable;
}

int libamg_comcerto_get_vad_enable(void)
{
	int vad_enable = 0;
	struct libamg_comcerto_config *config = libamg_comcerto_parse_config();

	vad_enable = config->vad_enable;
	free(config);

	return vad_enable;
}

char * libamg_comcerto_get_codec_g723_1_bitrate_name(void)
{
	struct libamg_comcerto_config *config = libamg_comcerto_parse_config();

	if (config->codec_g723_1){
		free(config);
		return G723_1_BITRATE_63_NAME;
	}

	free(config);
	return G723_1_BITRATE_53_NAME;
}

int libamg_comcerto_get_codec_passthrough_code(const char *codec_name)
{
	if (!strcmp(codec_name, CODEC_G711_A_NAME))
		return CODEC_G711_A_COD;
	else if (!strcmp(codec_name, CODEC_G711_U_NAME))
		return CODEC_G711_U_COD;
	else
		return 0;
}

char * libamg_comcerto_get_codec_passthrough_name(int codec_code)
{
	switch (codec_code) {
		case CODEC_G711_A_COD:
			return CODEC_G711_A_NAME;
			break;
		case CODEC_G711_U_COD:
			return CODEC_G711_U_NAME;
			break;
		default:
			return NULL;
			break;
	}

	return 0;
}

int libamg_comcerto_get_codec_packetization_interval(int codec_code)
{
	struct libamg_comcerto_config *config = libamg_comcerto_parse_config();
	int interval = 20;

	if (config == NULL)
		return interval;

	switch (codec_code) {
		case CODEC_G711_A_COD:
			interval = config->codecs_intvl.g711_a;
			break;
		case CODEC_G711_U_COD:
			interval = config->codecs_intvl.g711_u;
			break;
		case CODEC_G723_1_COD:
			interval = config->codecs_intvl.g723_1;
			break;
		case CODEC_G726_16Kbps_COD:
			interval = config->codecs_intvl.g726_16;
			break;
		case CODEC_G726_24Kbps_COD:
			interval = config->codecs_intvl.g726_24;
			break;
		case CODEC_G726_32Kbps_COD:
			interval = config->codecs_intvl.g726_32;
			break;
		case CODEC_G726_40Kbps_COD:
			interval = config->codecs_intvl.g726_40;
			break;
		case CODEC_G729_COD:
			interval = config->codecs_intvl.g729;
			break;
		case CODEC_GSM_COD:
			interval = config->codecs_intvl.gsm;
			break;
		default:
			break;
	}

	free(config);

	return interval;
}

int libamg_comcerto_reset_config(void)
{
	char command[BUF_SIZE];

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "cp %s%s %s", FILE_CONCERTO_CONF_DEFAULT_PATH, FILE_COMCERTO_CONF_NAME, FILE_COMCERTO_CONF_PATH);

	return system(command);
}

int libamg_comcerto_rtp_reload(void)
{
	return system("asterisk -rx \"module reload librtp-comcerto\"");
}

int libamg_comcerto_set_debugging(void)
{
	struct libamg_comcerto_config *config = libamg_comcerto_parse_config();

	/* If any debug is enabled, raise verbose level */
	if (config->debug)
		system("/sbin/asterisk -rx 'core set verbose 99'");

	if (config->debug & DEBUG_SIP)
		system("/sbin/asterisk -rx 'sip set debug on'");
	if (config->debug & DEBUG_MFCR2)
		system("/sbin/asterisk -rx 'mfcr2 set debug all'");
	if (config->debug & DEBUG_ISDN)
		system("/sbin/asterisk -rx 'pri set debug on span 1'");
	if (config->debug & DEBUG_DTMF)
		system("/sbin/asterisk -rx 'core set debug 99'");

	return 0;
}

struct libamg_comcerto_config *libamg_comcerto_parse_config(void)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *key;
	char *value;
	struct libamg_comcerto_config *conf;

	/* Alloc config struct */
	conf = malloc(sizeof(struct libamg_comcerto_config));
	if (!conf) {
		libamg_log_error("Error allocating memory\n")
		return NULL;
	}
	memset(conf, 0, sizeof(struct libamg_comcerto_config));

	/* Open Comcerto config file */
	file = fopen(FILE_COMCERTO_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return NULL;
	}

	/* Load Comcerto config parameters */
	while (fgets(buffer, BUF_SIZE, file)) {
		/* Parse key and value */
		key = buffer;
		if (strchr(key, '=') == NULL)
			continue;
		value = libamg_str_next_token(buffer, '=');

		/* Crop trailing commentary */
		strtok(value, " \t\n\0;#");

		/* Parse parameters */
		if (!strcmp(key, "codec_g723_1")) {
			conf->codec_g723_1 = !strcmp(value, "6.3kbps");
		} else if (!strcmp(key, "pass_through")) {
			conf->pass_through = libamg_comcerto_get_codec_passthrough_code(value);
		} else if (!strcmp(key, "ais_enable")) {
			conf->ais_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "vad_enable")) {
			conf->vad_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "vad_level")) {
			conf->vad_level = atoi(value);
		} else if (!strcmp(key, "cng_enable")) {
			conf->cng_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "echocancel_enable")) {
			conf->echocan_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "txgain")) {
			conf->txgain = atoi(value);
		} else if (!strcmp(key, "rxgain")) {
			conf->rxgain = atoi(value);
		} else if (!strcmp(key, "ectail")) {
			conf->ectail = atoi(value);
		} else if (!strcmp(key, "e1_enable")) {
			conf->e1_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "e1_loopback_enable")) {
			conf->e1_loopback_enable = !strcmp(value, "yes");
		/* Jitter Buffer configs */
		} else if (!strcmp(key, "jbenable")) {
			conf->jb_conf.jb_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "jbmaxsize")) {
			conf->jb_conf.jb_maxsize = atoi(value);
		} else if (!strcmp(key, "jbimpl")) {
			conf->jb_conf.jb_impl = !strcmp(value, "adaptive");
		} else if (!strcmp(key, "jbmindelay")) {
			conf->jb_conf.jb_mindelay = atoi(value);
		} else if (!strcmp(key, "jbtypdelay")) {
			conf->jb_conf.jb_typdelay = atoi(value);
		} else if (!strcmp(key, "jbmaxdelay")) {
			conf->jb_conf.jb_maxdelay = atoi(value);
		} else if (!strcmp(key, "jbdelet_thrld")) {
			conf->jb_conf.jb_delet_thrld = atoi(value);
		} else if (!strcmp(key, "mfcr2_tone_amp")) {
			conf->mfcr2_tone_amp = atoi(value);
		} else if (!strcmp(key, "debug")) {
			conf->debug = atoi(value);
		}
				/* CODECS INTERVAL*/
		else if (!strcmp(key, "codec_intvl_g711_a")) {
			conf->codecs_intvl.g711_a = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g711_u")) {
			conf->codecs_intvl.g711_u = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g723_1")) {
			conf->codecs_intvl.g723_1 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g726_16")) {
			conf->codecs_intvl.g726_16 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g726_24")) {
			conf->codecs_intvl.g726_24 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g726_32")) {
			conf->codecs_intvl.g726_32 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g726_40")) {
			conf->codecs_intvl.g726_40 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_g729")) {
			conf->codecs_intvl.g729 = atoi(value);
		} else if (!strcmp(key, "codec_intvl_gsm")) {
			conf->codecs_intvl.gsm = atoi(value);
		}

	}

	fclose(file);

	return conf;
}

int libamg_comcerto_save_config(struct libamg_comcerto_config *conf)
{
	FILE *file;
	struct libamg_sip_config * sip_conf;

	/* Open Comcerto config file */
	file = fopen(FILE_COMCERTO_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Tag needed by Asterisk */
	fprintf(file, "[general]\n");

	/* Save Comcerto conf. parameters */
	fprintf(file, "debug=%d\n", conf->debug);
	fprintf(file, "ais_enable=%s\n", conf->ais_enable ? "yes" : "no");
	fprintf(file, "vad_enable=%s\n", conf->vad_enable ? "yes" : "no");
	fprintf(file, "vad_level=%hd\n", conf->vad_level);
	fprintf(file, "cng_enable=%s\n", conf->cng_enable ? "yes" : "no");
	fprintf(file, "echocancel_enable=%s\n", conf->echocan_enable ? "yes" : "no");
	fprintf(file, "ectail=%hd\n", conf->ectail);
	fprintf(file, "e1_enable=%s\n", conf->e1_enable ? "yes" : "no");
	fprintf(file, "e1_loopback_enable=%s\n", conf->e1_loopback_enable ? "yes" : "no");
	fprintf(file, "mfcr2_tone_amp=%d\n", conf->mfcr2_tone_amp);
	/* Jitter Buffer configs */
	fprintf(file, "jbenable=%s\n", "yes"); /* ALWAYS ON FOR COMCERTO*/
	fprintf(file, "jbmaxsize=%hd\n", conf->jb_conf.jb_maxsize);
	fprintf(file, "jbimpl=%s\n", conf->jb_conf.jb_impl ? "adaptive" : "fixed");
	fprintf(file, "jbmindelay=%hd\n", conf->jb_conf.jb_mindelay);
	fprintf(file, "jbtypdelay=%hd\n", conf->jb_conf.jb_typdelay);
	fprintf(file, "jbmaxdelay=%hd\n", conf->jb_conf.jb_maxdelay);
	fprintf(file, "jbdelet_thrld=%hd\n", conf->jb_conf.jb_delet_thrld);
	/* Tx/Rx Gain */
	fprintf(file, "txgain=%hd\n",conf->txgain);
	fprintf(file, "rxgain=%hd\n",conf->rxgain);
	/* G723.1 codec config */
	fprintf(file, "codec_g723_1=%s\n", conf->codec_g723_1 ? "6.3kbps" : "5.3kbps");
	/* Pass-through config */
	if (conf->pass_through)
		fprintf(file, "pass_through=%s\n", libamg_comcerto_get_codec_passthrough_name(conf->pass_through));

	/* Codecs Intervals */
	fprintf(file, "codec_intvl_g711_a=%hd\n",conf->codecs_intvl.g711_a);
	fprintf(file, "codec_intvl_g711_u=%hd\n",conf->codecs_intvl.g711_u);
	fprintf(file, "codec_intvl_g723_1=%hd\n",conf->codecs_intvl.g723_1);
	fprintf(file, "codec_intvl_g726_16=%hd\n",conf->codecs_intvl.g726_16);
	fprintf(file, "codec_intvl_g726_24=%hd\n",conf->codecs_intvl.g726_24);
	fprintf(file, "codec_intvl_g726_32=%hd\n",conf->codecs_intvl.g726_32);
	fprintf(file, "codec_intvl_g726_40=%hd\n",conf->codecs_intvl.g726_40);
	fprintf(file, "codec_intvl_g729=%hd\n",conf->codecs_intvl.g729);
	fprintf(file, "codec_intvl_gsm=%hd\n",conf->codecs_intvl.gsm);

	fclose(file);

	/* Saves Jitter Buffer configs inside SIP_conf file*/

	/* Retrieve SIP configs*/
	sip_conf = libamg_sip_parse_config();

	/* Add Jitter buffer confs in SIP conf */
	sip_conf->jb_conf = conf->jb_conf;

	/* Save new SIP confs*/
	if (libamg_sip_save_config(sip_conf) < 0)
		return -1;

	return 0;
}
