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
 *  File name: sys_log.c
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

#include "sys_log.h"
#include "log.h"
#include "str.h"

#define BUF_SIZE	256

int libamg_sys_log_daemon_apply_config(void)
{
	struct libamg_sys_log_config *config = libamg_sys_log_parse_config();
	char command[BUF_SIZE];
	int length = 0;


	system("/bin/pkill syslogd");
	memset(command, 0, sizeof(command));
	length += sprintf(command, "%s", DAEMON_SYS_LOG);

	if (config->remote_enable){
		if (config->loglevel)
			length += sprintf(command+length, " -l %d", config->loglevel);
		if (strlen(config->remote_host) > 0)
			length += sprintf(command+length, " -L -R %s:%d", config->remote_host, DAEMON_PORT_SYS_LOG);
	}

	system(command);

	free(config);

	return 0;
}

int libamg_sys_log_reset_config(void)
{
	char command[BUF_SIZE];

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), "cp %s%s %s", FILE_SYS_LOG_CONF_DEFAULT_PATH, FILE_SYS_LOG_CONF_NAME, FILE_SYS_LOG_CONF_PATH);

	return system(command);
}

struct libamg_sys_log_config *libamg_sys_log_parse_config(void)
{
	FILE *file;
	char buffer[BUF_SIZE];
	char *key;
	char *value;
	struct libamg_sys_log_config *conf;

	/* Alloc config struct */
	conf = malloc(sizeof(struct libamg_sys_log_config));
	if (!conf) {
		libamg_log_error("Error allocating memory\n")
		return NULL;
	}
	memset(conf, 0, sizeof(struct libamg_sys_log_config));

	/* Open IP config file */
	file = fopen(FILE_SYS_LOG_CONF, "r");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return NULL;
	}

	/* Load sys_log config and add CGI parameters */
	while (fgets(buffer, BUF_SIZE, file)) {
		/* Parse key and value */
		key = buffer;
		if (strchr(key, '=') == NULL)
			continue;
		value = libamg_str_next_token(buffer, '=');

		/* Crop trailing commentary */
		strtok(value, " \t\n\0;#");

		/* Parse parameters */
			/* General confs */
		if (!strcmp(key, "remote_enable")) {
			conf->remote_enable = atoi(value);
		} else if (!strcmp(key, "loglevel")) {
			conf->loglevel = atoi(value);
		} else if (!strcmp(key, "remote_host")) {
			strncpy(conf->remote_host, value, 63);
		}
	}
	fclose(file);

	return conf;
}

int libamg_sys_log_save_config(struct libamg_sys_log_config *conf)
{
	FILE *file;

	/* Open sys_log config file */
	file = fopen(FILE_SYS_LOG_CONF, "w");
	if (file == NULL) {
		libamg_log_error("Error opening file\n");
		return -1;
	}

	fprintf(file, "remote_enable=%d\n", conf->remote_enable);

	fprintf(file, "loglevel=%d\n", conf->loglevel);

	fprintf(file, "remote_host=%s\n", conf->remote_host);

	fclose(file);

	return 0;
}
