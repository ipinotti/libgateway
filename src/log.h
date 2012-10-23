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
 * File name: log.h
 * Created on: Aug 27, 2012
 * Author: Igor Pinotti
 *
 * Definitions for system logging
 */

#ifndef LOG_H_
#define LOG_H_

#include <errno.h>
#include <syslog.h>

/*
 * Log Function
 */
#define DEBUG_LIBAMG_SYSLOG

#if defined (DEBUG_LIBAMG_SYSLOG)
#define libamg_log(x,...) \
		syslog(LOG_INFO,  "%s : %d => "x, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#elif defined (DEBUG_LIBAMG_PRINTF)
#define libamg_log(x,...) \
		printf("%s : %d => "x, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define libamg_log(x,...)
#endif

#define libamg_log_error(x,...) \
	syslog(LOG_ERR, "%s:%d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif /* LOG_H_ */
