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
 *  File name: process.c
 *  Created on: Jul 19, 2012
 *  Author: Thomas Del Grande <tgrande@pd3.com.br>
 *
 *  Process management functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <dirent.h>
#include <time.h>
#include <syslog.h>

#include "process.h"

pid_t libamg_process_get_pid(char *progname)
{
	DIR *dir;
	struct dirent *d;
	FILE *fp;
	int pid = 0, found = 0, f, c;
	char *s, *q;
	char path[256];
	char buf[256];
	char cmdline[MAX_PROC_CMDLINE];

	/* Open the /proc directory. */
	if ((dir = opendir("/proc")) == NULL) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		return (0);
	}

	/* Walk through the directory. */
	while (((d = readdir(dir)) != NULL) && (!found)) {
		/* See if this is a process */
		if ((pid = atoi(d->d_name)) == 0)
			continue;

		/* Open the statistics file. */
		sprintf(path, "/proc/%s/stat", d->d_name);

		/* Read SID & statname from it. */
		if ((fp = fopen(path, "r")) != NULL) {
			buf[0] = 0;
			fgets(buf, 256, fp);
			fclose(fp);

			/* See if name starts with '(' */
			s = buf;
			while (*s != ' ')
				s++;

			s++;
			if (*s == '(') {
				/* Read program name. */
				q = strrchr(buf, ')');

				if (q == NULL)
					continue;

				*q = 0;
				s++;

				if (strcmp(s, progname) == 0) {
					/* Now read arguments */
					sprintf(path, "/proc/%s/cmdline", d->d_name);

					if ((fp = fopen(path, "r")) != NULL) {
						f = 0;

						while (f < (MAX_PROC_CMDLINE - 1) && (c = fgetc(fp)) != EOF) {
							cmdline[f++] = c ? c : ' ';
						}

						cmdline[f++] = 0;
						fclose(fp);
					} else {
						/* Process disappeared.. */
						continue;
					}

					found = 1;
				}
			}
		} else {
			/* Process disappeared.. */
			continue;
		}

	}

	closedir(dir);

	return (found ? pid : 0);
}


static int _telinit(char c, int sleeptime)
{
	struct init_request request;
	int fd, bytes;

	memset(&request, 0, sizeof(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_RUNLVL;
	request.runlevel = c;
	request.sleeptime = sleeptime;

	fd = open(INIT_FIFO, O_WRONLY);
	if (fd < 0) {
		syslog(LOG_ERR, "telinit");
		return -1;
	}

	bytes = write(fd, &request, sizeof(request));
	close(fd);

	return ((bytes == sizeof(request)) ? 0 : (-1));
}

static int _set_init_program(int enable, char *prog)
{
	FILE *f;
	char buf[256];
	int found = 0;

	if ((f = fopen(FILE_INITTAB, "r+")) == NULL) {
		syslog(LOG_ERR, "could not open %s", FILE_INITTAB);
		return (-1);
	}
	while ((!feof(f)) && (!found)) {
		fgets(buf, 255, f);
		buf[255] = 0;
		if (strstr(buf, prog)) {
			fseek(f, -strlen(buf), SEEK_CUR);
			fputc(enable ? 'u' : '#', f);
			found = 1;
		}
	}
	fclose(f);
	if (!found) {
		syslog(LOG_ERR, "%s not found in %s", prog, FILE_INITTAB);
		return (-1);
	}
	return _telinit('q', 5);
}

int libamg_process_start_init_program(char *prog)
{
	return _set_init_program(1, prog);
}

int libamg_process_stop_init_program(char *prog)
{
	return _set_init_program(0, prog);
}

int libamg_process_check_daemon(char *prog)
{
	FILE *f;
	char buf[256];
	int found = 0;
	int c = 0;

	f = fopen(FILE_INITTAB, "r+");
	if (!f) {
		syslog(LOG_ERR, "could not open %s", FILE_INITTAB);
		return (0);
	}
	while ((!feof(f)) && (!found)) {
		fgets(buf, 255, f);
		buf[255] = 0;
		if (strstr(buf, prog)) {
			fseek(f, -strlen(buf), SEEK_CUR);
			c = fgetc(f);
			found = 1;
		}
	}
	fclose(f);
	if (!found) {
		syslog(LOG_ERR, "%s not found in %s", prog, FILE_INITTAB);
		return (0);
	}
	return (c == 'u');
}

int libamg_process_spawn_daemon(char *path, ...)
{
	pid_t pid, sid;
	va_list ap;
	char *argv[MAX_ARGS];
	int i;

	argv[0] = path;
	va_start(ap, path);
	for (i = 1; i < MAX_ARGS; i++) {
		argv[i]=va_arg(ap, char *);
		if (argv[i] == NULL)
			break;
	}
	va_end(ap);
	argv[i] = NULL;

	/* Fork off the parent process */
	process_dbg("Forking to run %s\n", path);
	pid = fork();
	if (pid < 0)
		return -1;

	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0)
		return 0;


	/* At this point we are executing as the child process */
	/* Change the file mode mask */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory.  This prevents the current
	 directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	execv(path, argv);
	exit(EXIT_FAILURE); /* Not reached */
}

int libamg_process_run_prog(int no_out, char *path, ...)
{
	int ret, i;
	pid_t pid;
	va_list ap;
	char *argv[MAX_ARGS];

	argv[0] = path;
	va_start(ap, path);
	for (i = 1; i < MAX_ARGS; i++) {
		argv[i]=va_arg(ap, char *);
		if (argv[i] == NULL)
			break;
	}
	va_end(ap);
	argv[i] = NULL;
	process_dbg("Before fork with UID %d\n", geteuid());

	pid = fork();
	switch (pid) {
	case 0: /* child */
		process_dbg("Child running %s with UID %d\n", path, geteuid());
		if (no_out) {
			//close(1); // close stdout
			//close(2); // close stderr
			freopen("/dev/null", "w", stdout);
			freopen("/dev/null", "w", stderr);
		}
		ret = execv(path, argv);
		syslog(LOG_ERR,"%s : unable to execute %s ", strerror(errno), path);
		exit(-1);
	case -1:
		syslog(LOG_ERR,"could not fork");
		return (-1);
		break;

	default: /* parent */
	{
		int status;
		process_dbg("Parent running %s with UID %d\n", path, geteuid());
		if (waitpid(pid, &status, 0) < 0) {
			syslog(LOG_ERR,"waitpid");
			return (-1);
		}
		if (!WIFEXITED(status)) {
			syslog(LOG_ERR,"child did not exited normally");
			return (-1);
		}
		if (WEXITSTATUS(status) < 0) {
			syslog(LOG_ERR,"child exit status = %d %s",
			                WEXITSTATUS(status), path);
			return (-1);
		}
		return 0;
	}
		break;
	}
}

#define FILE_INETD "/etc/inetd.conf"

static int _process_set_inetd_program(int enable, char *prog)
{
	FILE *f;
	char buf[256];
	int found = 0;
	pid_t pid;

	f = fopen(FILE_INETD, "r+");
	if (!f) {
		syslog(LOG_ERR, "could not open %s", FILE_INETD);
		return (-1);
	}

	while ((!feof(f)) && (!found)) {
		fgets(buf, 255, f);
		buf[255] = 0;
		if (strstr(buf, prog)) {
			fseek(f, -strlen(buf), SEEK_CUR);
			fputc(enable ? ' ' : '#', f);
			found = 1;
		}
	}

	fclose(f);

	if (!found) {
		syslog(LOG_ERR, "%s not found in %s", prog, FILE_INETD);
		return (-1);
	}

	pid = libamg_process_get_pid("inetd");
	if (!pid) {
		syslog(LOG_ERR, "inetd is not running");
		return (-1);
	}
	kill(pid, SIGHUP);

	return 0;
}

int libamg_process_start_inetd_program(char *name)
{
	return _process_set_inetd_program(1, name);
}

int libamg_process_stop_inetd_program(char *name)
{
	return _process_set_inetd_program(0, name);
}


int libamg_process_check_inetd_program(char *prog)
{
	FILE *f;
	char buf[256];
	char *p;
	int found = 0;

	f = fopen(FILE_INETD, "r+");
	if (!f) {
		syslog(LOG_ERR, "could not open %s", FILE_INETD);
		return (-1);
	}

	while ((!feof(f)) && (!found)) {
		fgets(buf, 255, f);
		buf[255] = 0;
		p = buf;
		if ((*p != '#') && (strstr(buf, prog)))
			found = 1;
	}

	fclose(f);

	return (found);
}

int libamg_process_prog_capture(char *buffer, int len, char *path, ...)
{
	int ret, i;
	pid_t pid;
	va_list ap;
	char *argv[MAX_ARGS];
	int fd[2];
	int nbytes;
	char *buf;

	len--;
	argv[0] = path;
	va_start(ap, path);
	for (i = 1; i < MAX_ARGS; i++) {
		argv[i] = va_arg(ap, char *);
		if (argv[i] == NULL)
			break;
	}
	va_end(ap);
	argv[i] = NULL;
	/* Create pipe to communicate between two processes */
	pipe(fd);

	pid = fork();
	switch (pid) {
	case 0: /* child - execute program */
		close(fd[0]);
		close(1);
		close(2);

		/*
		 * This does the trick, the pipe file descriptor
		 * will be duplicated to the lowest available file
		 * descriptor, that is the ones we just closed :
		 * STDOUT and STDERR
		 */
		dup(fd[1]);
		dup(fd[1]);

		ret = execv(path, argv);
		syslog(LOG_ERR, "could not exec %s %s", path, argv);
		return -1;
		break;
	case -1:
		syslog(LOG_ERR, "fork");
		return -1;
		break;
	default: /* parent */
		close(fd[1]);
		buf = buffer;
		while (len > 0) {
			nbytes = read(fd[0], buf, len);
			if (nbytes > 0) {
				len -= nbytes;
				buf += nbytes;
			} else if (nbytes < 0) {
				perror("");
				exit(-1);
			} else
				break;
		}
		*buf = 0;
		waitpid(pid, 0, 0); // wait until child finishes
		return 1;
		break;
	}
}
