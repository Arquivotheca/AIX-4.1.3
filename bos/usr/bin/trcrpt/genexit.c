static char sccsid[] = "@(#)18  1.5  src/bos/usr/bin/trcrpt/genexit.c, cmdtrace, bos411, 9428A410j 10/29/93 05:20:27";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 * 
 * FUNCTIONS: genexit
 * 
 * ORIGINS: 83
 * 
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */



/* This routine is at the service of trcupdate.c. It releases the semaphore
 * of trcupdate routine (when the caller is trcrpt, there is no semaphore).
 * This routine compresses the file /etc/trcfmt if it was compressed before
 * (it's the case when the trace package is not installed).
 * This routine overloads the genexit() routine in raslib.
 */


#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

extern compressflg;
extern semaflg;
extern *Tmpltfile;
extern sid;

/*
 * Function: compress the file TMPLT_DFLT, set in main.c.
 *           release the semaphore and exit with exitcode
 * Inputs: exitcode
 * Output: none; exit on exitcode
 */

genexit(exitcode)
int	exitcode;
{
char	cmd[256];
struct sembuf sb;


	if (compressflg) {
	sprintf(cmd, "%s %s", "/usr/bin/compress", Tmpltfile);
	if (system(cmd) != 0) {
	/* Compress file */
		perror("compress");
		exitcode = 1;
	}
	}


	if (semaflg) {
		sb.sem_num = 0;
		sb.sem_op = 1;
		sb.sem_flg = 0;
		/* Release the semaphore */
		if (semop(sid, &sb, 1) == -1) {
			perror ("semoprel");
			exitcode = 1;
		}


	}


	exit(exitcode);
}


