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
 * File: dahdi.h
 * Created on: Aug 27, 2012
 * Author: Igor Pinotti <igorpinotti@pd3.com.br>
 * 		   Based on dahdi.c by Wagner Gegler
 *
 * DAHDI configuration functions
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

#include <dahdi/user.h>

#include "dahdi.h"
#include "log.h"
#include "str.h"

static int libamg_dahdi_indirect_ioctl(int span, int cmd, void *data)
{
	struct dahdi_indirect_data ind;
	int fd;
	int res;

	fd = open(FILE_DAHDI_IOCTL, O_RDWR);
	if (fd < 0) {
		return fd;
	}

	ind.chan = span * 31 + 1;
	ind.op = cmd;
	ind.data = data;

	res = ioctl(fd, DAHDI_INDIRECT, &ind);

	close(fd);

	return res;
}

static void _get_span_status_alarms(struct libamg_dahdi_span_status *span)
{
	if (span->stats.status & CTD_SPAN_STATUS_LOS) {
		span->alarms_str = "LoS";
	} else if (span->stats.status & CTD_SPAN_STATUS_AIS) {
		span->alarms_str = "AIS";
	} else if (span->stats.status & CTD_SPAN_STATUS_BFAE) {
		span->alarms_str = "BFAE";
	} else if (span->stats.status & CTD_SPAN_STATUS_MFAE) {
		span->alarms_str = "MFAE";
	} else if (span->stats.status & CTD_SPAN_STATUS_RAI) {
		span->alarms_str = "RAI";
	} else {
		span->alarms_str = "OK";
	}
}

char * libamg_dahdi_get_switchtype_name(int switchtype_code)
{
	switch (switchtype_code) {
		case DAHDI_SWITCHT_EURO_ISDN_COD:
			return DAHDI_SWITCHT_EURO_ISDN_NAME;
			break;
		case DAHDI_SWITCHT_NATIONAL_ISDN2_COD:
			return DAHDI_SWITCHT_NATIONAL_ISDN2_NAME;
			break;
		case DAHDI_SWITCHT_NORTEL_DMS100_COD:
			return DAHDI_SWITCHT_NORTEL_DMS100_NAME;
			break;
		case DAHDI_SWITCHT_ATeT_4ESS_COD:
			return DAHDI_SWITCHT_ATeT_4ESS_NAME;
			break;
		case DAHDI_SWITCHT_LUCENT_5ESS_COD:
			return DAHDI_SWITCHT_LUCENT_5ESS_NAME;
			break;
		case DAHDI_SWITCHT_NATIONAL_ISDN_1_COD:
			return DAHDI_SWITCHT_NATIONAL_ISDN_1_NAME;
			break;
		case DAHDI_SWITCHT_Q_SIG_COD:
			return DAHDI_SWITCHT_Q_SIG_NAME;
			break;
		default:
			return NULL;
			break;
	}

	return 0;
}

struct libamg_dahdi_status *libamg_dahdi_get_status(void)
{
	struct libamg_dahdi_status *status;
	int i;
	int res;

	/* Alloc struct */
	status = malloc(sizeof(struct libamg_dahdi_status));
	if (!status) {
		return status;
	}
	memset(status, 0, sizeof(struct libamg_dahdi_status));

	for (i = 0; i < NUMSPANS; ++i) {
		res = libamg_dahdi_indirect_ioctl(i, CTD_GET_SPAN_STATS,
				&status->spans[i].stats);

		/* Generate Alarms string */
		_get_span_status_alarms(&status->spans[i]);

	}

	return status;
}

void libamg_dahdi_reset_stats(void)
{
	int i;

	for (i = 0; i < NUMSPANS; ++i) {
		libamg_dahdi_indirect_ioctl(i, CTD_RESET_SPAN_STATS, NULL);
	}
}

int libamg_dahdi_ais_get(int span)
{
	int i, ret;

	ret = libamg_dahdi_indirect_ioctl(span, CTD_SPAN_AIS_GET, &i);
	if( ret != 0)
		return ret;

	return (i != 0);
}

int libamg_dahdi_ais_set(int span, int enable)
{
	int i;
	i = (enable != 0);

	return libamg_dahdi_indirect_ioctl(span, CTD_SPAN_AIS_SET, &i);
}

int libamg_dahdi_parse_system_conf(struct libamg_dahdi_config *conf)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *token;
	int token_int;
	struct libamg_dahdi_span *span;
	/* Open file */
	file = fopen(FILE_DAHDI_SYSTEM_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Parse file */
	while (fgets(buffer, BUF_SIZE, file)) {
		/* Parse key and value */
		if (strncmp(buffer, "span=", 5))
			continue;

		token = libamg_str_next_token(buffer, '=');

		/* Crop trailing commentary */
		strtok(token, " \t\n;#");

		/* Get span number */
		token = strtok(token, ",");
		token_int = atoi(token);

		if (token_int < 1 || token_int > NUMSPANS)
			continue;
		span = &conf->spans[token_int - 1];

		/* Get clock source */
		token = strtok(NULL, ",");
		token_int = atoi(token);
		if (token_int < 0 || token_int > NUMSPANS)
			continue;
		span->clock = token_int;

		/* Get CRC option */
		token = strtok(NULL, ",");
		token = strtok(NULL, ",");
		token = strtok(NULL, ",");
		token = strtok(NULL, ",");
		if (token && !strcmp(token, "crc4")) {
			span->crc = 1;
		} else {
			span->crc = 0;
		}

		span->enable = 1;
	}

	fclose(file);

	return 0;
}

int _parse_chan_dahdi_channel_line(struct libamg_dahdi_config *conf,
                                   struct libamg_dahdi_span *span,
                                   char *line)
{
	int span_offset;
	int first = 0;
	int a = 0;
	int b = 0;
	int c = 0;
	int res;

	res = sscanf(line, "%d-%d,%d-%d", &first, &a, &b, &c);
	if (res == 0 || first <= 0)
		return -1;
	if (res == 1) {
		span->channels = 1;
	} else if (res == 2) {
		span->channels = a - first + 1;
	} else if (res == 3) {
		span->channels = b - first + 1;
	} else if (res == 4) {
		span->channels = c - first + 1;
	} else {
		return -1;
	}

	span_offset = (first - 1) / 31;

	memcpy(&conf->spans[span_offset], span, sizeof(*span));
	return 0;
}

int libamg_dahdi_parse_chan_dahdi_conf(struct libamg_dahdi_config *conf)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *key;
	char *value;
	struct libamg_dahdi_span span;

	/* Open file */
	file = fopen(FILE_CHAN_DAHDI_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Parse file */
	while (fgets(buffer, BUF_SIZE, file)) {
		/* Parse key and value */
		key = buffer;
		if (strchr(key, '=') == NULL)
			continue;
		value = libamg_str_next_token(buffer, '=');

		/* Crop trailing commentary */
		strtok(value, " \t\n;#");

		/* Parse parameters */
		if (!strcmp(key, "signalling")) {
			if (!strcmp(value, "pri_net")) {
				span.signalling = DAHDI_SIG_ISDN_NET;
			} else if (!strcmp(value, "pri_cpe")) {
				span.signalling = DAHDI_SIG_ISDN_CPE;
			} else {
				span.signalling = DAHDI_SIG_MFCR2;
			}
		} else if (!strcmp(key, "overlapdial")) {
			if (!strcmp(value, "no")) {
				span.isdn.overlapdial = 0;
			} else {
				span.isdn.overlapdial = 1;
			}
		} else if (!strcmp(key, "switchtype")) {
			strncpy(span.isdn.switchtype, value, 15);
		} else if (!strcmp(key, "mfcr2_get_ani_first")) {
			if (!strcmp(value, "yes")) {
				span.mfcr2.get_ani_first = 1;
			} else {
				span.mfcr2.get_ani_first = 0;
			}
		} else if (!strcmp(key, "mfcr2_max_ani")) {
			span.mfcr2.max_ani = atoi(value);
		} else if (!strcmp(key, "mfcr2_max_dnis")) {
			span.mfcr2.max_dnis = atoi(value);
		} else if (!strcmp(key, "mfcr2_allow_collect_calls")) {
			if (!strcmp(value, "no")) {
				span.mfcr2.allow_collect_calls = 0;
			} else {
				span.mfcr2.allow_collect_calls = 1;
			}
		} else if (!strcmp(key, "mfcr2_double_answer")) {
			if (!strcmp(value, "yes")) {
				span.mfcr2.double_answer = 1;
			} else {
				span.mfcr2.double_answer = 0;
			}
		} else if (!strcmp(key, "channel")) {
			value = libamg_str_next_token(value, '>');
			_parse_chan_dahdi_channel_line(conf, &span, value);
		} else if (!strcmp(key, "echocancel")) {
			if (!strcmp(value, "yes")) {
				span.echocancel = 1;
			} else {
				span.echocancel = 0;
			}
		}
	}

	fclose(file);

	return 0;
}

struct libamg_dahdi_config *libamg_dahdi_parse_config(void)
{
	struct libamg_dahdi_config *conf;

	/* Alloc config struct */
	conf = malloc(sizeof(struct libamg_dahdi_config));
	if (!conf) {
		libamg_log_error("Error allocating memory\n")
		return NULL;
	}
	memset(conf, 0, sizeof(struct libamg_dahdi_config));

	/* Parse files */
	if (libamg_dahdi_parse_chan_dahdi_conf(conf)) {
		free(conf);
		return NULL;
	}
	if (libamg_dahdi_parse_system_conf(conf)) {
		free(conf);
		return NULL;
	}

	return conf;
}

void _gen_channels_line(char *line, int span, int channels)
{
	int offset = span * 31 + 1;

	if (channels == 1) {
		sprintf(line, "%d", offset);
	} else if (channels < 16) {
		sprintf(line, "%d-%d", offset, offset + channels - 1);
	} else if (channels == 16) {
		sprintf(line, "%d-%d,%d", offset, offset + 14, offset + 16);
	} else {
		sprintf(line, "%d-%d,%d-%d", offset, offset + 14,
				offset + 16, offset + channels);
	}
}

int libamg_dahdi_apply_system_conf(struct libamg_dahdi_config *conf)
{
	FILE *file;
	int i;
	char buffer[BUF_SIZE];
	struct libamg_dahdi_span *span;

	/* Open file */
	file = fopen(FILE_DAHDI_SYSTEM_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Save configuration */
	fprintf(file, "%s", DAHDI_SYSTEM_CONTENT);
	for (i = 0; i < NUMSPANS; ++i) {
		span = &conf->spans[i];
		if (!span->enable)
			continue;
		_gen_channels_line(buffer, i, 30);
		if (span->signalling == DAHDI_SIG_MFCR2) {
			fprintf(file, "span=%d,%d,0,cas,hdb3%s\n", i + 1,
					span->clock,
					span->crc ? ",crc4" : "");
			fprintf(file, "cas=%s:1101\n", buffer);
		} else {
			fprintf(file, "span=%d,%d,0,ccs,hdb3%s\n", i + 1,
					span->clock,
					span->crc ? ",crc4" : "");
			fprintf(file, "bchan=%s\n", buffer);
			fprintf(file, "dchan=%d\n", 31 * i + 16);
		}
	}

	fclose(file);

	return 0;
}

int libamg_dahdi_save_chan_dahdi_conf(struct libamg_dahdi_config *conf)
{
	FILE *file;
	int i;
	char buffer[BUF_SIZE];
	struct libamg_dahdi_span *span;

	/* Open file */
	file = fopen(FILE_CHAN_DAHDI_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Save chan_dahdi configuration */
	fprintf(file, "%s", CHAN_DAHDI_CONTENT);

	for (i = 0; i < NUMSPANS; ++i) {
		span = &conf->spans[i];
		if (!span->enable)
			continue;

		/* Signalling */
		if (span->signalling == DAHDI_SIG_ISDN_CPE) {
			fprintf(file, "signalling=pri_cpe\n");
		} else if (span->signalling == DAHDI_SIG_ISDN_NET) {
			fprintf(file, "signalling=pri_net\n");
		} else {
			fprintf(file, "signalling=mfcr2\n");
		}

		/* Signalling parameters */
		fprintf(file, "overlapdial=%s\n",
				span->isdn.overlapdial ? "yes" : "no");
		fprintf(file, "switchtype=%s\n", span->isdn.switchtype);
		fprintf(file, "mfcr2_get_ani_first=%s\n",
			span->mfcr2.get_ani_first ? "yes" : "no");
		fprintf(file, "mfcr2_max_ani=%d\n", span->mfcr2.max_ani);
		fprintf(file, "mfcr2_max_dnis=%d\n", span->mfcr2.max_dnis);
		fprintf(file, "mfcr2_allow_collect_calls=%s\n",
			span->mfcr2.allow_collect_calls ? "yes" : "no");
		fprintf(file, "mfcr2_double_answer=%s\n",
			span->mfcr2.double_answer ? "yes" : "no");

		/* Context */
		fprintf(file, "context=from-span%d\n", i + 1);

		/* Group */
		fprintf(file, "group=%d\n", i + 1);

		/* Channels */
		_gen_channels_line(buffer, i, conf->spans[i].channels);
		fprintf(file, "channel=>%s\n", buffer);

		/* Echo Cancel */
		fprintf(file, "echocancel=%s\n",
				span->echocancel ? "yes" : "no");
	}

	fclose(file);

	return 0;
}

int libamg_dahdi_save_config(struct libamg_dahdi_config *conf)
{
	int ret;

	ret = libamg_dahdi_apply_system_conf(conf);
	if (ret)
		return ret;

	ret = libamg_dahdi_save_chan_dahdi_conf(conf);
	if (ret)
		return ret;

	return 0;
}

int libamg_ais_daemon_check(void)
{
	char buffer[BUF_SIZE];
	FILE *file;
	int reti, ret_val;
	regex_t regex;
	regmatch_t matches[4];

	/* Open file */
	file = fopen(FILE_AISDAEMON_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}
	reti = regcomp(&regex, "^host_keepalive=\\([a-zA-Z]*\\)", 0);
	if( reti != 0 ) {
		fclose(file);
		return -1;
	}

	/* Parse file */
	while (fgets(buffer, BUF_SIZE-1, file)) {
		ret_val = 0;
		reti = regexec(&regex, buffer, sizeof(matches)/sizeof(regmatch_t), matches, 0);
		if( reti != 0 )
			continue;
		if( strncasecmp(&buffer[matches[1].rm_so], "yes", 3) == 0 ) {
			ret_val = 1;
			break;
		} else if( strncasecmp(&buffer[matches[1].rm_so], "no", 2) == 0 ) {
			ret_val = 0;
			break;
		} else {
			ret_val = 0;
			break;
		}
	}

	fclose(file);
	regfree(&regex);

	return ret_val;
}

int libamg_ais_daemon_set(int enable)
{
	FILE *file;

	/* Open file */
	file = fopen(FILE_AISDAEMON_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}
	/* Save chan_dahdi configuration */
	fprintf(file, "host_keepalive=%s\n", (enable ? "yes": "no"));
	fclose(file);
	return 0;
}

int libamg_ais_daemon_apply_config(void)
{
	system("/bin/killall -1 aisd");
	return 0;
}

void libamg_dahdi_apply_config(void)
{
	/* Apply new configuration */

	/* Hack for unloading Comcerto Module - Any ideas ? */
	system("/sbin/asterisk -rx 'module unload libcomcerto-config.so'");
	system("/sbin/asterisk -rx 'core stop now'");
	system("/sbin/dahdi_cfg");
	system("/sbin/asterisk");
}

void libamg_dahdi_flash_save_config(void)
{
	/* Create directories if they don't exist */
	system("if [ ! -d /mnt/config/asterisk ]; then "
			"mkdir /mnt/config/asterisk; fi");
	system("if [ ! -d /mnt/config/dahdi ]; then "
			"mkdir /mnt/config/dahdi; fi");

	/* Save configuration to flash */
	system("cp " FILE_DAHDI_SYSTEM_CONF " /mnt/config/dahdi");
	system("cp " FILE_AISDAEMON_CONF " /mnt/config/dahdi");
	system("cp " FILE_CHAN_DAHDI_CONF " /mnt/config/asterisk");
}
