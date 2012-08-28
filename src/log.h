/*
 * log.h
 *
 *  Created on: Aug 27, 2012
 *      Author: Igor Pinotti
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
