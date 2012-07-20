/*
 * process.h
 *
 *  Created on: Jul 19, 2012
 *      Author: tgrande
 */

#ifndef PROCESS_H_
#define PROCESS_H_

//#define PROCESS_DEBUG
#ifdef PROCESS_DEBUG
#define process_dbg(x,...)         syslog(LOG_INFO, "%s : %d => "x , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define process_dbg(x,...)
#endif


#include <sys/param.h>

#define INIT_MAGIC 0x03091969
#define INIT_FIFO  "/dev/initctl"
#define INIT_CMD_START         0
#define INIT_CMD_RUNLVL        1
#define INIT_CMD_POWERFAIL     2
#define INIT_CMD_POWERFAILNOW  3
#define INIT_CMD_POWEROK       4

struct init_request {
	int magic; /* Magic number                 */
	int cmd; /* What kind of request         */
	int runlevel; /* Runlevel to change to        */
	int sleeptime; /* Time between TERM and KILL   */
	char gen_id[8]; /* Beats me.. telnetd uses "fe" */
	char tty_id[16]; /* Tty name minus /dev/tty      */
	char host[MAXHOSTNAMELEN]; /* Hostname                     */
	char term_type[16]; /* Terminal type                */
	int signal; /* Signal to send               */
	int pid; /* Process to send to           */
	char exec_name[128]; /* Program to execute           */
	char reserved[128]; /* For future expansion.        */
};

#define MAX_PROC_CMDLINE   256

#define MAX_ARGS 10

#define FILE_INITTAB "/etc/inittab"

/**
 * Get process PID
 *
 * @param progname Process name
 * @return Process PID if found, 0 otherwise
 */
pid_t libamg_process_get_pid(char *progname);


/**
 * Start a program run by init
 *
 * @param prog : What to run
 * @return 0 if success, -1 otherwise
 */
int libamg_process_start_init_program(char *prog);

/**
 * Stop a program run by init
 *
 * @param prog : What to run
 * @return 0 if success, -1 otherwise
 */
int libamg_process_stop_init_program(char *prog);

/**
 * Check if an init daemon is running according to inittab
 *
 * @param prog: Program name
 * @return 1 if running, 0 otherwise
 */
int libamg_process_check_init_daemon(char *prog);

/**
 * Spawn a process daemon-like (on background)
 *
 * @param path: What to run
 * @return 0 if success, -1 if error
 */
int libamg_process_spawn_daemon(char *path, ...);

/**
 * Executes a program
 *
 * @param no_out: Disable process output
 * @param path  : What to run
 * @return 0 if success, -1 if error
 */
int libamg_process_run_prog(int no_out, char *path, ...);

/**
 * Enable service run by inetd
 *
 * @param prog   : Service name
 * @return 0 if success, -1 if error
 */
int libamg_process_start_inetd_program(char *prog);

/**
 * Disable service run by inetd
 *
 * @param prog   : Service name
 * @return 0 if success, -1 if error
 */
int libamg_process_stop_inetd_program(char *prog);

/**
 * Check if program is active according to inetd configuration
 *
 * @param prog Program name
 * @return 1 if running, 0 if not
 */
int libamg_process_check_inetd_program(char *prog);

/**
 * Execute a process and store STDOUT and STDERR inside buffer
 *
 * @note This function blocks until the child exits
 *
 * @param buffer : Where to store
 * @param len    : Buffer length
 * @param path   : What to execute
 * @return
 */
int libamg_process_prog_capture(char *buffer, int len, char *path, ...);

#endif /* PROCESS_H_ */
