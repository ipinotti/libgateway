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
 *  File name: system.c
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

#include "log.h"
#include "str.h"
#include "system.h"

#define BUF_SIZE 64


char *libamg_get_system_version(void)
{
	FILE *file;
	static char fw_version[BUF_SIZE];
	memset(fw_version, 0, 33);

	/* Open IP config file */
	file = fopen(FILE_SYSTEM_VERSION, "r");
	if (file == NULL) {
		libamg_log_error("Error opening Firmware Version file\n");
		return NULL;
	}

	fgets(fw_version, BUF_SIZE, file);

	fclose(file);

	return fw_version;
}

