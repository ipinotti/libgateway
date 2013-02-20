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

#define BUF_SIZE 256

char *libamg_system_get_version(void)
{
	FILE *file;
	static char fw_version[BUF_SIZE];
	memset(fw_version, 0, sizeof(fw_version));

	/* Open IP config file */
	file = fopen(FILE_SYSTEM_VERSION, "r");
	if (file == NULL) {
		libamg_log_error("Error opening Firmware Version file.\n");
		return NULL;
	}

	fgets(fw_version, BUF_SIZE, file);

	fclose(file);

	return fw_version;
}

char *libamg_system_get_memory(void)
{
	int i;
	FILE *file;
	static char memory[BUF_SIZE];
	char buf[BUF_SIZE];
	memset(memory, 0, sizeof(memory));
	memset(buf, 0, sizeof(buf));

	file = fopen(FILE_SYSTEM_MEMORY, "r");
	if (file == NULL) {
		libamg_log_error("Error opening memory info file.\n");
		return NULL;
	}

	for (i = 0; (i < 2) && !feof(file); i++) {
		if (fgets(buf, sizeof(buf), file)) {
			strcat(memory, buf);
		}
	}

	fclose(file);

	return memory;
}

char *libamg_system_get_uptime(void)
{
	FILE *p_file;
	static char uptime[BUF_SIZE];
	memset(uptime, 0, sizeof(BUF_SIZE));

	p_file = popen(FILE_SYSTEM_UPTIME, "r");

	if (p_file == NULL) {
		libamg_log_error("Error reading system Uptime.\n");
		return NULL;
	}

	fgets(uptime, BUF_SIZE, p_file);

	if (p_file)
		pclose(p_file);

	return uptime;
}


char *libamg_system_get_cpu_usage(void)
{
	FILE *tf;
	float cpu;
	long long idle, user, nice, system, iowait, irq, softirq;
	static long long idle_old = 0, nice_old = 0, user_old = 0, system_old = 0;
	static long long iowait_old = 0, irq_old = 0, softirq_old = 0;
	static char cpu_usage[BUF_SIZE];
	memset(cpu_usage, 0, sizeof(BUF_SIZE));
	float scale;
	/* enough for a /proc/stat CPU line (not the intr line) */
	char buf[BUF_SIZE];

	tf = fopen(FILE_SYSTEM_CPU_USAGE, "r");
	if (tf == NULL) {
		libamg_log_error("Error opening cpu usage file.\n");
		return NULL;
	}

	if (tf) {
		fgets(buf, sizeof(buf), tf);
		if (sscanf(buf, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu", &user, &nice, &system, &idle, &iowait,
		                &irq, &softirq) == 7) {
			scale = 100.0 / (float) ((user - user_old) + (nice - nice_old)
			                + (system - system_old) + (idle - idle_old) + (iowait - iowait_old)
			                + (irq - irq_old) + (softirq - softirq_old));

			cpu = (float) ((user - user_old) + (nice - nice_old) + (system - system_old)
			                + (iowait - iowait_old) + (irq - irq_old) + (softirq - softirq_old))
			                * scale;
#if 0
			pprintf ("processor usage : %#5.1f%% user, %#5.1f%% system, %#5.1f%% nice, %#5.1f%% idle\n"
					"\t%#5.1f%% iowait, %#5.1f%% irq, %#5.1f%% softirq\n",
					(float)(user-user_old)*scale, (float)(system-system_old)*scale,
					(float)(nice-nice_old)*scale, (float)(idle-idle_old)*scale,
					(float)(iowait-iowait_old)*scale, (float)(irq-irq_old)*scale,
					(float)(softirq-softirq_old)*scale);

#else
			sprintf(cpu_usage, "%0.1f%% system, %0.1f%% idle", cpu,
			                (float) ((idle - idle_old) * scale));
#endif

			user_old = user;
			nice_old = nice;
			system_old = system;
			idle_old = idle;
			iowait_old = iowait;
			irq_old = irq;
			softirq_old = softirq;
		}
		fclose(tf);
	}

	return cpu_usage;
}

char *libamg_system_get_cpu_info(void)
{
	FILE *file;
	static char cpu_info[4*BUF_SIZE];
	char buffer[BUF_SIZE];
	memset(cpu_info, 0, sizeof(cpu_info));
	memset(buffer, 0, sizeof(buffer));

	/* Open IP config file */
	file = fopen(FILE_SYSTEM_CPU_INFO, "r");
	if (file == NULL) {
		libamg_log_error("Error opening cpu info file.\n");
		return NULL;
	}

	while (fgets(buffer, sizeof(buffer), file)) {
		strcat(cpu_info, buffer);
	}

	fclose(file);

	return cpu_info;
}

int libamg_system_get_serialnum(char *data, int maxlen)
{
	char buffer[4096];
	char *p, *q;
	int fd, n;

	memset(data, 0, maxlen);

	fd = open(FILE_SYSTEM_SERIAL_NUM, O_RDONLY);
	if (fd < 0) {
		libamg_log_error("Error opening serialnum info file.\n");
		return -1;
	}

	n = read(fd, buffer, sizeof(buffer));
	if (n <= 0)
		return -1;

	/* Find start of serial number string */
	p = strstr(buffer, "serialnum=");
	if (p == NULL)
		return -1;

	p += strlen("serialnum=");

	/* End the string when a space is found */
	q = p;
	while(*q != 0) {
		if (*q == ' ') {
			*q = 0;
			break;
		}
		q++;
	}

	strncpy(data, p, maxlen);

	return 0;
}
