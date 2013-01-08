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
 *  File name: sys_log.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti <igorpinotti@pd3.com.br>
 *
 */

#ifndef SYS_LOG_H_
#define SYS_LOG_H_


/*
 * General Includes
 */



/*
 * General Defines
 */

#define FILE_SYS_LOG_CONF_NAME			"sys_log.conf"
#define FILE_SYS_LOG_CONF_PATH			"/etc/"
#define FILE_SYS_LOG_CONF				FILE_SYS_LOG_CONF_PATH FILE_SYS_LOG_CONF_NAME
#define FILE_SYS_LOG_CONF_DEFAULT_PATH	"/etc.ro/"

#define DAEMON_SYS_LOG		"/sbin/syslogd"

#define DAEMON_PORT_SYS_LOG 172


/*
 * General Structures
 */

struct libamg_sys_log_config {
	int remote_enable;
	int loglevel;
	char remote_host[64];
};


/*
 * Functions Declaration
 */

/**
 * libamg_sys_log_daemon_apply_config
 *
 * Apply syslog daemon configs.
 *
 * @return 0 if ok
 */
int libamg_sys_log_daemon_apply_config(void);

/**
 * libamg_sys_log_reset_config
 *
 * Restore the sys_log configures from default sys_log configuration file
 *
 * @return 0 if success, negative if error
 */
int libamg_sys_log_reset_config(void);

/**
 * libamg_sys_log_parse_config
 *
 * Get sys_log configs through libamg_sys_log_config structure.
 *
 * @return struct libamg_sys_log_config *
 */
struct libamg_sys_log_config *libamg_sys_log_parse_config(void);

/**
 * libamg_sys_log_save_config
 *
 * Saves sys_log configs at libamg_sys_log_config structure
 * inside sys_log configuration file.
 *
 * @param conf
 * @return -1 if error, 0 if OK
 */
int libamg_sys_log_save_config(struct libamg_sys_log_config *conf);

#endif /* SYS_LOG_H_ */
