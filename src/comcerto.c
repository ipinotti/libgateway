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
		} else if (!strcmp(key, "loopback_enable")) {
			conf->loopback_enable = !strcmp(value, "yes");
		}
	}

	fclose(file);

	return conf;
}

int libamg_comcerto_save_config(struct libamg_comcerto_config *conf)
{
	FILE *file;

	/* Open Comcerto config file */
	file = fopen(FILE_COMCERTO_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Save Comcerto conf. parameters */

	fprintf(file, "vad_enable=%s\n", conf->vad_enable ? "yes" : "no");
	fprintf(file, "vad_level=%hd\n", conf->vad_level);
	fprintf(file, "cng_enable=%s\n", conf->cng_enable ? "yes" : "no");
	fprintf(file, "ectail=%hd\n", conf->ectail);
	fprintf(file, "loopback_enable=%s\n", conf->loopback_enable ? "yes" : "no");

	fclose(file);

	return 0;
}
