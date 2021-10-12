static char sccsid[] = "@(#)96	1.2  src/bos/usr/bin/errlg/libras/savebase.c, cmderrlg, bos411, 9428A410j 6/22/93 15:30:49";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: savebase
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <errno.h>
#include <sys/types.h>

extern Progname;

/*
 * NAME:      savebase
 * FUNCTION:  Synchronize the data saved to the Customize data base with
 *            what is on the boot record.
 * RETURNS:   None
 */

/* savebase
** - This routine sync's the data saved to the Customize data base with
** what is on the boot record.
** - Unfortunatly, there is not a library interface to this program, but
** only an executable living in /etc.  Hence we have to fork() and exec()
** this program.
*/
#define SAVEBASE "/usr/sbin/savebase"
savebase()
{
	pid_t pid, wpid;

	if((pid = fork()) == 0) {
		execl(SAVEBASE,SAVEBASE,0);
		perror(SAVEBASE);
		exit(1);
		
	}
	else if (pid == -1) {
		perror(Progname);
		exit(1);
	}
e_intr:
	wpid = wait(0);
	if (((wpid < 0) && (errno == EINTR)) || ((wpid != pid) && (wpid > 0)))
		goto e_intr;
}
