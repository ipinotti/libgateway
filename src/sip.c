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
 *  File name: sip.c
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *      Based on sip.c by Wagner Gegler
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

#include "sip.h"
#include "log.h"
#include "str.h"

#define BUF_SIZE	256

struct libamg_sip_config *libamg_sip_parse_config(void)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *key;
	char *value;
	struct libamg_sip_config *conf;
	struct libamg_sip_account *account;

	/* Alloc config struct */
	conf = malloc(sizeof(struct libamg_sip_config));
	if (!conf) {
		libamg_log_error("Error allocating memory\n")
		return NULL;
	}
	memset(conf, 0, sizeof(struct libamg_sip_config));
	account = &conf->accounts[0];

	/* Open IP config file */
	file = fopen(FILE_SIP_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return NULL;
	}

	/* Load SIP config and add CGI parameters */
	while (fgets(buffer, BUF_SIZE, file)) {
		/* Parse key and value */
		key = buffer;
		if (strchr(key, '=') == NULL)
			continue;
		value = libamg_str_next_token(buffer, '=');

		/* Crop trailing commentary */
		strtok(value, " \t\n;#");

		/* Parse parameters */
			/* General confs */
		if (!strcmp(key, "bindport")) {
			conf->bindport = atoi(value);
		} else if (!strcmp(key, "jbenable")) {
			conf->jb_conf.jb_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "jbmaxsize")) {
			conf->jb_conf.jb_maxsize = atoi(value);
		} else if (!strcmp(key, "jbimpl")) {
			conf->jb_conf.jb_impl = !strcmp(value, "adaptative");
#ifdef NOT_YET_SUPPORTED
		} else if (!strcmp(key, "jbmindelay")) {
			conf->jb_conf.jb_mindelay = atoi(value);
		} else if (!strcmp(key, "jbtypdelay")) {
			conf->jb_conf.jb_typdelay = atoi(value);
		} else if (!strcmp(key, "jbmaxdelay")) {
			conf->jb_conf.jb_maxdelay = atoi(value);
		} else if (!strcmp(key, "jbdelet_thrld")) {
			conf->jb_conf.jb_delet_thrld = atoi(value);
#endif
		} else if (!strcmp(key, "register")) {
			conf->register_enable = 1;
			/* TODO get other parameters... */

			/* Account confs*/
		} else if (!strcmp(key, "host")) {
			strncpy(account->host, value, 63);
		} else if (!strcmp(key, "port")) {
			account->port = atoi(value);
		} else if (!strcmp(key, "username")) {
			strncpy(account->username, value, 63);
		} else if (!strcmp(key, "secret")) {
			strncpy(account->secret, value, 63);
		} else if (!strcmp(key, "dtmfmode")) {
			strncpy(account->dtmfmode, value, 31);
		}
#ifdef ABANDONED
		else if (!strcmp(key, "allow")) {
			if (!strcmp(value, CODEC_G711_A))
				account->allow.g711_A = 1;
			else if (!strcmp(value, CODEC_G711_U))
					account->allow.g711_U = 1;
			else if (!strcmp(value, CODEC_G723_1))
					account->allow.g723_1 = 1;
			else if (!strcmp(value, CODEC_G726_16Kbps))
					account->allow.g726_16kbps = 1;
			else if (!strcmp(value, CODEC_G726_24Kbps))
					account->allow.g726_24kbps = 1;
			else if (!strcmp(value, CODEC_G726_32Kbps))
					account->allow.g726_32kbps = 1;
			else if (!strcmp(value, CODEC_G726_40Kbps))
					account->allow.g726_40kbps = 1;
			else if (!strcmp(value, CODEC_G729))
					account->allow.g729 = 1;
			else if (!strcmp(value, CODEC_GSM))
					account->allow.gsm = 1;
		}
#endif
		 else if (!strcmp(key, "nat")) {
			account->nat = !strcmp(value, "yes");
		} else if (!strcmp(key, "qualify")) {
			account->qualify = !strcmp(value, "yes");
		} else if (!strcmp(key, "insecure")) {
			strncpy(account->insecure, value, 31);
		} else if (!strcmp(key, "callerid")) {
			strncpy(account->callerid, value, 63);
		} else if (!strcmp(key, "fromuser")) {
			strncpy(account->fromuser, value, 63);
		}
	}

	fclose(file);

	return conf;
}

int libamg_sip_save_config(struct libamg_sip_config *conf)
{
	FILE *file;
	struct libamg_sip_account *account = &conf->accounts[0];

	/* Open SIP config file */
	file = fopen(FILE_SIP_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Save SIP general default parameters */
	fprintf(file, "%s", SIP_GENERAL_CONTENT);

	/* Jitter Buffer Confs (Same as Comcerto confs) */
	fprintf(file, "jbenable=%s\n", "yes"); /* ALWAYS ON FOR COMCERTO*/
	fprintf(file, "jbmaxsize=%hd\n", conf->jb_conf.jb_maxsize);
	fprintf(file, "jbimpl=%s\n", conf->jb_conf.jb_impl ? "adaptative" : "fixed");
#ifdef NOT_YET_SUPPORTED
	fprintf(file, "jbmindelay=%hd\n", conf->jb_conf.jb_mindelay);
	fprintf(file, "jbtypdelay=%hd\n", conf->jb_conf.jb_typdelay);
	fprintf(file, "jbmaxdelay=%hd\n", conf->jb_conf.jb_maxdelay);
	fprintf(file, "jbdelet_thrld=%hd\n", conf->jb_conf.jb_delet_thrld);
#endif

	/* Save SIP Useragent */
	fprintf(file, "useragent=" PRODUCT_NAME "\n");

	/* Save SIP bindport */
	fprintf(file, "bindport=%hd\n", conf->bindport);

#ifdef NOT_SUPPORTED_AMG
	/* Save SIP account registration */
	if (account->register_enable) {
		/* register=>user:pass@sip_host:sip_port */
		fprintf(file, "register=>%s:%s@%s:%hd\n",
				account->username, account->secret,
				account->host, account->port);
	}
	fprintf(file, "callerid=%s\n", account->callerid);
	if (strlen(account->fromuser) > 0)
		fprintf(file, "fromuser=%s\n", account->fromuser);
	fprintf(file, "nat=%s\n", account->nat ? "yes" : "no");
	fprintf(file, "qualify=%s\n", account->qualify ? "yes" : "no");
	fprintf(file, "insecure=%s\n", account->insecure);
#endif

	/* Save SIP account parameters */
	fprintf(file, "%s", SIP_ACCOUNT_CONTENT);
	fprintf(file, "host=%s\n", account->host);
	fprintf(file, "port=%hd\n", account->port);
	fprintf(file, "username=%s\n", account->username);
	fprintf(file, "secret=%s\n", account->secret);
	fprintf(file, "dtmfmode=%s\n", account->dtmfmode);

#ifdef ABANDONED
	if (account->allow.g711_A)
		fprintf(file, "allow=%s\n", CODEC_G711_A);
	if (account->allow.g711_U)
		fprintf(file, "allow=%s\n", CODEC_G711_U);
	if (account->allow.g723_1)
		fprintf(file, "allow=%s\n", CODEC_G723_1);
	if (account->allow.g726_16kbps)
		fprintf(file, "allow=%s\n", CODEC_G726_16Kbps);
	if (account->allow.g726_24kbps)
		fprintf(file, "allow=%s\n", CODEC_G726_24Kbps);
	if (account->allow.g726_32kbps)
		fprintf(file, "allow=%s\n", CODEC_G726_32Kbps);
	if (account->allow.g726_40kbps)
		fprintf(file, "allow=%s\n", CODEC_G726_40Kbps);
	if (account->allow.g729)
		fprintf(file, "allow=%s\n", CODEC_G729);
	if (account->allow.gsm)
		fprintf(file, "allow=%s\n", CODEC_GSM);
#endif

	fclose(file);

	return 0;
}

void libamg_sip_flash_save_config(void)
{
	/* Create dir if needed */
	system("if [ ! -d /mnt/config/asterisk ]; then "
			"mkdir /mnt/config/asterisk; fi");

	/* Save configuration on flash*/
	system("cp " FILE_SIP_CONF " /mnt/config/asterisk");
}

int libamg_asterisk_get_register(char *response, int maxlen)
{
	regex_t regex;
	regmatch_t matches[10];
	int reti;
	FILE *pipe;
	char buffer[BUF_SIZE];

	pipe = popen("/sbin/asterisk -rx \"sip show registry\"", "r");
	if( pipe == NULL )
		return -1;

	reti = regcomp(&regex, "^[[:digit:].:]+[[:blank:]]+[A-Z][[:blank:]]+[[:alnum:]]+[[:blank:]]+[[:digit:]]+[[:blank:]]+([[:alpha:]]+[ ]?[[:alpha:]]*)", REG_EXTENDED);
	if( reti != 0 ) {
		fclose(pipe);
		return -1;
	}

	while (feof(pipe) == 0) {
		/* Read line from file */
		if( fgets(buffer, BUF_SIZE, pipe) == 0 )
			break;

		reti = regexec(&regex, buffer, sizeof(matches)/sizeof(regmatch_t), matches, 0);
		if( reti != 0 )
			continue;

		if (matches[1].rm_so > 0 && (matches[1].rm_eo-matches[1].rm_so) > 0 ) {
			buffer[matches[1].rm_eo] = '\0';
			strncpy(response, &buffer[matches[1].rm_so], maxlen);
		}
	}

	fclose(pipe);
	regfree(&regex);
	return 0;
}
