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
 *  File name: extensions.c
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *
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

#include "extensions.h"
#include "log.h"
#include "str.h"

#define BUF_SIZE	256

int libamg_extensions_reset_config(void)
{
	char command[BUF_SIZE];

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "cp %s%s %s", FILE_EXTENSIONS_CONF_DEFAULT_PATH, FILE_EXTENSIONS_CONF_NAME, FILE_EXTENSIONS_CONF_PATH);

	return system(command);
}

struct libamg_extensions_config *libamg_extensions_parse_config(void)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *key;
	char *value;
	struct libamg_extensions_config *conf;

	/* Alloc config struct */
	conf = malloc(sizeof(struct libamg_extensions_config));
	if (!conf) {
		libamg_log_error("Error allocating memory\n")
		return NULL;
	}
	memset(conf, 0, sizeof(struct libamg_extensions_config));

	/* Open IP config file */
	file = fopen(FILE_EXTENSIONS_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return NULL;
	}

	/* Load extensions config and add CGI parameters */
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
		if (!strcmp(key, "PREFIX")) {
			conf->prefix = atoi(value);
			conf->prefix_enable = 1;
		}
	}
	fclose(file);

	return conf;
}

int libamg_extensions_save_config(struct libamg_extensions_config *conf)
{
	FILE *file;

	/* Open extensions config file */
	file = fopen(FILE_EXTENSIONS_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	/* Save extensions general default parameters */
	fprintf(file, "%s", EXTENSIONS_GENERAL_CONTENT);

	/* Save extensions globals parameters */
	fprintf(file, "%s", EXTENSIONS_GLOBALS_CONTENT);
	if (conf->prefix_enable)
		fprintf(file, "%s=%hd\n", EXTENSIONS_GLOBALS_PREFIX, conf->prefix);

	/* Save extensions account (from-span1) default parameters */
	fprintf(file, "%s", EXTENSIONS_ACCOUNT_FROM_SPAN1_CONTENT);
	if (conf->prefix_enable)
		fprintf(file, "%s", EXTENSIONS_ACCOUNT_FROM_SPAN1_DIAL_PREFIX);
	else
		fprintf(file, "%s", EXTENSIONS_ACCOUNT_FROM_SPAN1_DIAL_STAND);

	/* Save extensions account (nop) default parameters */
	fprintf(file, "%s", EXTENSIONS_ACCOUNT_NOP_CONTENT);

	/* Save extensions account (from-sip) default parameters */
	fprintf(file, "%s", EXTENSIONS_ACCOUNT_FROM_SIP_CONTENT);

	fclose(file);

	return 0;
}

void libamg_extensions_flash_save_config(void)
{
	/* Create dir if needed */
	system("if [ ! -d /mnt/config/asterisk ]; then "
			"mkdir /mnt/config/asterisk; fi");

	/* Save configuration on flash*/
	system("cp " FILE_EXTENSIONS_CONF " /mnt/config/asterisk");
}
