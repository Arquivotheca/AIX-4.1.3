static char sccsid[] = "@(#)19  1.5  src/bos/usr/bin/trcrpt/handfmt.c, cmdtrace, bos411, 9428A410j 1/17/94 06:19:46";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 * 
 * FUNCTIONS: main
 * 
 * ORIGINS: 83
 * 
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

/* This program is used during the installation (or the removing) of the
 * package trace. It is called by trace.config shell script.
 * The handfmt() routine gets a semaphore and compress()'es (if remove) or
 * uncompress()'es (if it's installation) the  file /etc/trcfmt
 */


#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/trcmacros.h>

#define TRCFMT "/etc/trcfmt"
#define TRCFMT_Z "/etc/trcfmt.Z"
#define SEMA_KEY 16893671
#define SYSTRACE "systrace"
#define SYSTRCTL "systrctl"

int	sid;

main(argc, argv)
char	*argv[];
{
	struct sembuf sb;


	if (*argv[1] == '0')
		reldevice();

	if ((sid = semget(SEMA_KEY, 1, 0666)) == -1) {
		if (errno == ENOENT) {   /* the semaphore not yet exist  */ 
			if ((sid = semget(SEMA_KEY, 1, 0666 | IPC_CREAT )) == -1) 
				exit(1);
		} else
			exit(1);
	} else {
		sb.sem_num = 0;
		sb.sem_op = -1;
		sb.sem_flg = 0;
		if (semop(sid, &sb, 1) == -1)
			exit(1);
	}


	if (*argv[1] == '1')
		config_uncomp();
	else
		config_comp();

}


/*
 * Function: releases special files SYSTRACE, SYSTRACE?, SYSTRCTL and
 *           SYSTRCTL?
 * Inputs: none
 * Outputs: none
 */

reldevice()
{
	char	devname[52];
	int	i;

	reldevno(SYSTRACE, TRUE);
	reldevno(SYSTRCTL, TRUE);

	for (i = 1 ; i < TRC_NCHANNELS; i++) {
		sprintf(devname, "%s%d", SYSTRACE, i);
		reldevno(devname, TRUE);
		sprintf(devname, "%s%d", SYSTRCTL, i);
		reldevno(devname, TRUE);
	}
}


/*
 * Function: stat()'es /etc/trcfmt file and uncompresses it.
 * Inputs: none
 * Outputs: none; if any error, config_exit()'es
 */
config_uncomp()
{
	char	cmd[256];
	struct stat buf;

	if (stat(TRCFMT, &buf) != 0) {
		if (errno != ENOENT)
			config_exit(1);
	} else
		config_exit(0);


	if (stat(TRCFMT_Z, &buf) != 0) {
		if (errno == ENOENT)
			config_exit(0);
		else
			config_exit(1);
	}


	sprintf(cmd, "%s %s %s", "/usr/bin/uncompress", TRCFMT, "2>/dev/null");
	if (system(cmd) != 0)
		config_exit(1);
	else
		config_exit(0);
}


/*
 * Function: stat()'es /etc/trcfmt file and compresses it with option -F.
 * Inputs: none
 * Outputs: none; if any error, config_exit()'es
 */
config_comp()
{
	char	cmd[256];
	struct stat buf;

	if (stat(TRCFMT, &buf) != 0) {
		if (errno == ENOENT)
			config_exit(0);
		else
			config_exit(1);
	}


	sprintf(cmd, "%s %s %s", "/usr/bin/compress -F", TRCFMT, "2>/dev/null");
	if (system(cmd) != 0)
		config_exit(1);
	else
		config_exit(0);
}



/*
 * Function: releases the semaphore that was grabbed
 * Inputs: exitcode
 * Outputs: none; if any error, exit()'es
 */

config_exit(exitcode)
int	exitcode;
{
	struct sembuf sb;

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = 0;
	/* Release the semaphore */
	if (semop(sid, &sb, 1) == -1)
		exitcode = 1;

	exit(exitcode);
}


