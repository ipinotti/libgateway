/*
 * comcerto.c
 *
 *  Created on: Sep 5, 2012
 *      Author: Igor Kramer Pinotti
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
		strtok(value, " \t\n;#");

		/* Parse parameters */
		if (!strcmp(key, "vad_enable")) {
			conf->vad_enable = !strcmp(value, "yes");
		} else if (!strcmp(key, "vad_level")) {
			conf->vad_level = atoi(value);
		} else if (!strcmp(key, "cng_enable")) {
			conf->cng_enable = !strcmp(value, "yes");
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
			conf->jb_conf.jb_impl = !strcmp(value, "adaptative");
		} else if (!strcmp(key, "jbmindelay")) {
			conf->jb_conf.jb_mindelay = atoi(value);
		} else if (!strcmp(key, "jbtypdelay")) {
			conf->jb_conf.jb_typdelay = atoi(value);
		} else if (!strcmp(key, "jbmaxdelay")) {
			conf->jb_conf.jb_maxdelay = atoi(value);
		} else if (!strcmp(key, "jbdelet_thrld")) {
			conf->jb_conf.jb_delet_thrld = atoi(value);
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
	fprintf(file, "[general]\n\n");

	/* Save Comcerto conf. parameters */
	fprintf(file, "vad_enable=%s\n", conf->vad_enable ? "yes" : "no");
	fprintf(file, "vad_level=%hd\n", conf->vad_level);
	fprintf(file, "cng_enable=%s\n", conf->cng_enable ? "yes" : "no");
	fprintf(file, "ectail=%hd\n", conf->ectail);
	fprintf(file, "e1_enable=%s\n", conf->e1_enable ? "yes" : "no");
	fprintf(file, "e1_loopback_enable=%s\n", conf->e1_loopback_enable ? "yes" : "no");
	/* Jitter Buffer configs */
	fprintf(file, "jbenable=%s\n", conf->jb_conf.jb_enable ? "yes" : "no"); /* ALWAYS ON FOR COMCERTO*/
	fprintf(file, "jbmaxsize=%hd\n", conf->jb_conf.jb_maxsize);
	fprintf(file, "jbimpl=%s\n", conf->jb_conf.jb_impl ? "adaptative" : "fixed");
	fprintf(file, "jbmindelay=%hd\n", conf->jb_conf.jb_mindelay);
	fprintf(file, "jbtypdelay=%hd\n", conf->jb_conf.jb_typdelay);
	fprintf(file, "jbmaxdelay=%hd\n", conf->jb_conf.jb_maxdelay);
	fprintf(file, "jbdelet_thrld=%hd\n", conf->jb_conf.jb_delet_thrld);

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
