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
 *  File name: system.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

/*
 * General Defines
 */

#define FILE_SYSTEM_VERSION 	"/etc/fw_version"
#define FILE_SYSTEM_UPTIME		"/bin/uptime"
#define FILE_SYSTEM_MEMORY		"/proc/meminfo"
#define FILE_SYSTEM_CPU_USAGE 	"/proc/stat"
#define FILE_SYSTEM_CPU_INFO	"/proc/cpuinfo"

/*
 * Functions Declaration
 */

/**
 * libamg_system_get_version
 *
 * Get system firmware version from file in FILE_SYSTEM_VERSION
 *
 * @return string
 */
char *libamg_system_get_version(void);

/**
 * libamg_system_get_memory
 *
 * Get system memory info stored in FILE_SYSTEM_MEMORY
 *
 * @return string
 */
char *libamg_system_get_memory(void);

/**
 * libamg_system_get_uptime
 *
 * Get system uptime stored in FILE_SYSTEM_UPTIME
 *
 * @return string
 */
char *libamg_system_get_uptime(void);

/**
 * libamg_system_get_cpu_usage
 *
 * Get system CPU usage stored in FILE_SYSTEM_CPU_USAGE
 *
 * @return string
 */
char *libamg_system_get_cpu_usage(void);

/**
 * libamg_system_get_cpu_info
 *
 * Get system CPU info stored in FILE_SYSTEM_CPU_INFO
 *
 * @return string
 */
char *libamg_system_get_cpu_info(void);

#endif /* SYSTEM_H_ */
